/* This file is part of the KDE Project
   Copyright (c) 2007-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2013 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "filewatch.h"
#include "metadatamover.h"
#include "fileindexerconfig.h"
#include "activefilequeue.h"
#include "regexpcache.h"
#include "database.h"

#ifdef BUILD_KINOTIFY
#include "kinotify.h"
#endif

#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtDBus/QDBusConnection>

#include <QDebug>
#include <KConfigGroup>

#ifdef BUILD_KINOTIFY
namespace
{
/**
 * A variant of KInotify which ignores all paths in the Nepomuk
 * ignore list.
 */
class IgnoringKInotify : public KInotify
{
public:
    IgnoringKInotify(Baloo::FileIndexerConfig* config, QObject* parent);
    ~IgnoringKInotify();

protected:
    bool filterWatch(const QString& path, WatchEvents& modes, WatchFlags& flags);

private:
    Baloo::FileIndexerConfig* m_config;
};

IgnoringKInotify::IgnoringKInotify(Baloo::FileIndexerConfig* config, QObject* parent)
    : KInotify(parent)
    , m_config(config)
{
}

IgnoringKInotify::~IgnoringKInotify()
{
}

//
// Non-indexed directories are never watched as their metadata is stored
// in the extended attributes for the file as is never lost. Unless your filesystem
// does not support xattr, then you're out of luck. Sorry.
//
bool IgnoringKInotify::filterWatch(const QString& path, WatchEvents&, WatchFlags&)
{
    return m_config->shouldFolderBeIndexed(path);
}
}
#endif // BUILD_KINOTIFY

using namespace Baloo;

FileWatch::FileWatch(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
#ifdef BUILD_KINOTIFY
    , m_dirWatch(0)
#endif
{
    m_metadataMover = new MetadataMover(m_db, this);
    connect(m_metadataMover, SIGNAL(movedWithoutData(QString)),
            this, SLOT(slotMovedWithoutData(QString)));
    connect(m_metadataMover, SIGNAL(fileRemoved(int)),
            this, SIGNAL(fileRemoved(int)));

    m_fileModificationQueue = new ActiveFileQueue(this);
    connect(m_fileModificationQueue, SIGNAL(urlTimeout(QString)),
            this, SLOT(slotActiveFileQueueTimeout(QString)));

#ifdef BUILD_KINOTIFY
    // monitor the file system for changes (restricted by the inotify limit)
    m_dirWatch = new IgnoringKInotify(m_config, this);

    connect(m_dirWatch, SIGNAL(moved(QString,QString)),
            this, SLOT(slotFileMoved(QString,QString)));
    connect(m_dirWatch, SIGNAL(deleted(QString,bool)),
            this, SLOT(slotFileDeleted(QString,bool)));
    connect(m_dirWatch, SIGNAL(created(QString,bool)),
            this, SLOT(slotFileCreated(QString,bool)));
    connect(m_dirWatch, SIGNAL(closedWrite(QString)),
            this, SLOT(slotFileClosedAfterWrite(QString)));
    connect(m_dirWatch, SIGNAL(watchUserLimitReached(QString)),
            this, SLOT(slotInotifyWatchUserLimitReached(QString)));
    connect(m_dirWatch, SIGNAL(installedWatches()),
            this, SIGNAL(installedWatches()));

    // Watch all indexed folders
    QStringList folders = m_config->includeFolders();
    Q_FOREACH (const QString& folder, folders) {
        watchFolder(folder);
    }
#else
    connectToKDirNotify();
#endif

    connect(m_config, SIGNAL(configChanged()),
            this, SLOT(updateIndexedFoldersWatches()));
}


FileWatch::~FileWatch()
{
}


// FIXME: listen to Create for folders!
void FileWatch::watchFolder(const QString& path)
{
    qDebug() << path;
#ifdef BUILD_KINOTIFY
    if (m_dirWatch && !m_dirWatch->watchingPath(path))
        m_dirWatch->addWatch(path,
                             KInotify::WatchEvents(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf | KInotify::EventCloseWrite | KInotify::EventCreate),
                             KInotify::WatchFlags());
#endif
}

void FileWatch::slotFileMoved(const QString& urlFrom, const QString& urlTo)
{
    m_metadataMover->moveFileMetadata(urlFrom, urlTo);
}


void FileWatch::slotFilesDeleted(const QStringList& paths)
{
    m_metadataMover->removeFileMetadata(paths);
}


void FileWatch::slotFileDeleted(const QString& urlString, bool isDir)
{
    // Directories must always end with a trailing slash '/'
    QString url = urlString;
    if (isDir && url[ url.length() - 1 ] != '/') {
        url.append('/');
    }
    slotFilesDeleted(QStringList(url));
}


void FileWatch::slotFileCreated(const QString& path, bool isDir)
{
    // we only need the file creation event for folders
    // file creation is always followed by a CloseAfterWrite event
    if (isDir) {
        Q_EMIT indexFile(path);
    }
}

void FileWatch::slotFileClosedAfterWrite(const QString& path)
{
    QDateTime current = QDateTime::currentDateTime();
    QDateTime fileModification = QFileInfo(path).lastModified();
    QDateTime dirModification = QFileInfo(QFileInfo(path).absoluteDir().absolutePath()).lastModified();

    // If we have received a FileClosedAfterWrite event, then the file must have been
    // closed within the last minute.
    // This is being done cause many applications open the file under write mode, do not
    // make any modifications and then close the file. This results in us getting
    // the FileClosedAfterWrite event
    if (fileModification.secsTo(current) <= 1000 * 60 ||
            dirModification.secsTo(current) <= 1000 * 60) {
        m_fileModificationQueue->enqueueUrl(path);
    }
}

void FileWatch::connectToKDirNotify()
{
    // monitor KIO for changes
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FileMoved",
                                          this, SIGNAL(slotFileMoved(QString,QString)));
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FilesRemoved",
                                          this, SIGNAL(slotFilesDeleted(QStringList)));
}


#ifdef BUILD_KINOTIFY

#include <syslog.h>
#include <kauth.h>

//Try to raise the inotify watch limit by executing
//a helper which modifies /proc/sys/fs/inotify/max_user_watches
bool raiseWatchLimit()
{
    // FIXME: vHanda: FIX ME!!
    return false;
    /*
    KAuth::Action limitAction("org.kde.baloo.filewatch.raiselimit");
    limitAction.setHelperID("org.kde.baloo.filewatch")

    KAuth::ActionReply reply = limitAction.execute();
    if (reply.failed()) {
        return false;
    }
    return true;
    */
}

//This slot is connected to a signal emitted in KInotify when
//inotify_add_watch fails with ENOSPC.
void FileWatch::slotInotifyWatchUserLimitReached(const QString& path)
{
    if (raiseWatchLimit()) {
        qDebug() << "Successfully raised watch limit, re-adding " << path;
        if (m_dirWatch)
            m_dirWatch->resetUserLimit();
        watchFolder(path);
    } else {
        //If we got here, we hit the limit and couldn't authenticate to raise it,
        // so put something in the syslog so someone notices.
        syslog(LOG_USER | LOG_WARNING, "KDE Baloo Filewatch service reached the inotify folder watch limit. File changes may be ignored.");
        // we do it the brutal way for now hoping with new kernels and defaults this will never happen
        // Delete the KInotify and switch to KDirNotify dbus signals
        if (m_dirWatch) {
            m_dirWatch->deleteLater();
            m_dirWatch = 0;
        }
        connectToKDirNotify();
        Q_EMIT installedWatches();
    }
}
#endif


void FileWatch::updateIndexedFoldersWatches()
{
#ifdef BUILD_KINOTIFY
    if (m_dirWatch) {
        QStringList folders = m_config->includeFolders();
        Q_FOREACH (const QString& folder, folders) {
            m_dirWatch->removeWatch(folder);
            watchFolder(folder);
        }
    }
#endif
}

void FileWatch::slotActiveFileQueueTimeout(const QString& url)
{
    qDebug() << url;
    Q_EMIT indexFile(url);
}

void FileWatch::slotMovedWithoutData(const QString& url)
{
    Q_EMIT indexFile(url);
}

