/* This file is part of the KDE Project
   Copyright (c) 2007-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

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
#include "pendingfilequeue.h"
#include "regexpcache.h"
#include "database.h"
#include "pendingfile.h"
#include "baloodebug.h"

#include "kinotify.h"

#include <QDir>
#include <QDateTime>
#include <QDBusConnection>

#include <syslog.h>

using namespace Baloo;

FileWatch::FileWatch(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
    , m_dirWatch(nullptr)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    m_metadataMover = new MetadataMover(m_db, this);
    connect(m_metadataMover, &MetadataMover::movedWithoutData, this, &FileWatch::indexNewFile);
    connect(m_metadataMover, &MetadataMover::fileRemoved, this, &FileWatch::fileRemoved);

    m_pendingFileQueue = new PendingFileQueue(this);
    connect(m_pendingFileQueue, &PendingFileQueue::indexNewFile, this, &FileWatch::indexNewFile);
    connect(m_pendingFileQueue, &PendingFileQueue::indexModifiedFile, this, &FileWatch::indexModifiedFile);
    connect(m_pendingFileQueue, &PendingFileQueue::indexXAttr, this, &FileWatch::indexXAttr);
    connect(m_pendingFileQueue, &PendingFileQueue::removeFileIndex, m_metadataMover, &MetadataMover::removeFileMetadata);

    // monitor the file system for changes (restricted by the inotify limit)
    m_dirWatch = new KInotify(m_config, this);

    connect(m_dirWatch, &KInotify::moved, this, &FileWatch::slotFileMoved);
    connect(m_dirWatch, &KInotify::deleted, this, &FileWatch::slotFileDeleted);
    connect(m_dirWatch, &KInotify::created, this, &FileWatch::slotFileCreated);
    connect(m_dirWatch, &KInotify::modified, this, &FileWatch::slotFileModified);
    connect(m_dirWatch, &KInotify::closedWrite, this, &FileWatch::slotFileClosedAfterWrite);
    connect(m_dirWatch, &KInotify::attributeChanged, this, &FileWatch::slotAttributeChanged);
    connect(m_dirWatch, &KInotify::watchUserLimitReached, this, &FileWatch::slotInotifyWatchUserLimitReached);
    connect(m_dirWatch, &KInotify::installedWatches, this, &FileWatch::installedWatches);
}


FileWatch::~FileWatch()
{
}

void FileWatch::watchIndexedFolders()
{
    // Watch all indexed folders
    QStringList folders = m_config->includeFolders();
    Q_FOREACH (const QString& folder, folders) {
        watchFolder(folder);
    }
}

// FIXME: listen to Create for folders!
void FileWatch::watchFolder(const QString& path)
{
    qCDebug(BALOO) << path;
    if (m_dirWatch && !m_dirWatch->watchingPath(path)) {
        KInotify::WatchEvents flags(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf
                                    | KInotify::EventCloseWrite | KInotify::EventCreate
                                    | KInotify::EventAttributeChange | KInotify::EventModify);

        m_dirWatch->addWatch(path, flags, KInotify::WatchFlags());
    }
}

void FileWatch::slotFileMoved(const QString& urlFrom, const QString& urlTo)
{
    m_metadataMover->moveFileMetadata(urlFrom, urlTo);
}


void FileWatch::slotFileDeleted(const QString& urlString, bool isDir)
{
    // Directories must always end with a trailing slash '/'
    QString url = urlString;
    if (isDir) {
        Q_ASSERT(!url.endsWith('/'));
        url.append(QLatin1Char('/'));
    }

    PendingFile file(url);
    file.setDeleted();

    m_pendingFileQueue->enqueue(file);
}


void FileWatch::slotFileCreated(const QString& path, bool isDir)
{
    Q_UNUSED(isDir);
    PendingFile file(path);
    file.setCreated();

    m_pendingFileQueue->enqueue(file);
}

void FileWatch::slotFileModified(const QString& path)
{
    PendingFile file(path);
    file.setModified();

    //qCDebug(BALOO) << "MOD" << path;
    m_pendingFileQueue->enqueue(file);
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
    if (fileModification.secsTo(current) <= 1000 * 60 || dirModification.secsTo(current) <= 1000 * 60) {
        PendingFile file(path);
        file.setClosedOnWrite();
        //qCDebug(BALOO) << "CLOSE" << path;
        m_pendingFileQueue->enqueue(file);
    }
}

void FileWatch::slotAttributeChanged(const QString& path)
{
    PendingFile file(path);
    file.setAttributeChanged();

    m_pendingFileQueue->enqueue(file);
}

// This slot is connected to a signal emitted in KInotify when
// inotify_add_watch fails with ENOSPC.
void FileWatch::slotInotifyWatchUserLimitReached(const QString&)
{
    //If we got here, we hit the limit and couldn't authenticate to raise it,
    // so put something in the syslog so someone notices.
    syslog(LOG_USER | LOG_WARNING, "KDE Baloo File Indexer has reached the inotify folder watch limit. File changes will be ignored.");
    // we do it the brutal way for now hoping with new kernels and defaults this will never happen
    // Delete the KInotify
    // FIXME: Maybe we should be aborting?
    if (m_dirWatch) {
        m_dirWatch->deleteLater();
        m_dirWatch = nullptr;
    }
    Q_EMIT installedWatches();
}

void FileWatch::updateIndexedFoldersWatches()
{
    if (m_dirWatch) {
        QStringList folders = m_config->includeFolders();
        Q_FOREACH (const QString& folder, folders) {
            m_dirWatch->removeWatch(folder);
            watchFolder(folder);
        }
    }
}

