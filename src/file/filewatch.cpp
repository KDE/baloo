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
#include "fileexcludefilters.h"
//#include "removabledeviceindexnotification.h"
//#include "removablemediacache.h"
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

#include <KDebug>
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
    IgnoringKInotify(RegExpCache* rec, QObject* parent);
    ~IgnoringKInotify();

protected:
    bool filterWatch(const QString& path, WatchEvents& modes, WatchFlags& flags);

private:
    RegExpCache* m_pathExcludeRegExpCache;
};

IgnoringKInotify::IgnoringKInotify(RegExpCache* rec, QObject* parent)
    : KInotify(parent),
      m_pathExcludeRegExpCache(rec)
{
}

IgnoringKInotify::~IgnoringKInotify()
{
}

bool IgnoringKInotify::filterWatch(const QString& path, WatchEvents& modes, WatchFlags& flags)
{
    Q_UNUSED(flags);

    QStringList cpts = path.split('/', QString::SkipEmptyParts);
    if (cpts.isEmpty())
        return false;

    // We only check the final componenet instead of all the components cause the
    // earlier ones would have already been tested by this function
    QString file = cpts.last();

    bool shouldFileNameBeIndexed = Baloo::FileIndexerConfig::self()->shouldFileBeIndexed(file);
    if (!shouldFileNameBeIndexed) {
        // If the path should not be indexed then we do not want to watch it
        // This is an optimization
        return false;
    }

    bool shouldFolderBeIndexed = Baloo::FileIndexerConfig::self()->folderInFolderList(path);

    // Only watch the index folders for file change.
    // We still need to monitor everything for file creation because directories count as
    // files, and we want to recieve signals for a new directory, so that we can watch it.
    if (shouldFolderBeIndexed && shouldFileNameBeIndexed) {
        modes |= KInotify::EventCloseWrite;
    } else {
        modes &= (~KInotify::EventCloseWrite);
    }
    return true;
}
}
#endif // BUILD_KINOTIFY

using namespace Baloo;

FileWatch::FileWatch(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
#ifdef BUILD_KINOTIFY
    , m_dirWatch(0)
#endif
{
    // Create the configuration instance singleton (for thread-safety)
    // ==============================================================
    (void)new FileIndexerConfig(this);

    // the list of default exclude filters we use here differs from those
    // that can be configured for the file indexer service
    // the default list should only contain files and folders that users are
    // very unlikely to ever annotate but that change very often. This way
    // we avoid a lot of work while hopefully not breaking the workflow of
    // too many users.
    m_pathExcludeRegExpCache = new RegExpCache();
    m_pathExcludeRegExpCache->rebuildCacheFromFilterList(defaultExcludeFilterList());

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
    m_dirWatch = new IgnoringKInotify(m_pathExcludeRegExpCache, this);

    connect(m_dirWatch, SIGNAL(moved(QString, QString)),
            this, SLOT(slotFileMoved(QString, QString)));
    connect(m_dirWatch, SIGNAL(deleted(QString, bool)),
            this, SLOT(slotFileDeleted(QString, bool)));
    connect(m_dirWatch, SIGNAL(created(QString, bool)),
            this, SLOT(slotFileCreated(QString, bool)));
    connect(m_dirWatch, SIGNAL(closedWrite(QString)),
            this, SLOT(slotFileClosedAfterWrite(QString)));
    connect(m_dirWatch, SIGNAL(watchUserLimitReached(QString)),
            this, SLOT(slotInotifyWatchUserLimitReached(QString)));
    connect(m_dirWatch, SIGNAL(installedWatches()),
            this, SIGNAL(installedWatches()));

    // recursively watch the whole home dir

    // FIXME: we cannot simply watch the folders that contain annotated files since moving
    // one of these files out of the watched "area" would mean we "lose" it, i.e. we have no
    // information about where it is moved.
    // On the other hand only using the homedir means a lot of restrictions.
    // One dummy solution would be a hybrid: watch the whole home dir plus all folders that
    // contain annotated files outside of the home dir and hope for the best

    const QString home = QDir::homePath();
    watchFolder(home);

    //Watch all indexed folders unless they are subdirectories of home, which is already watched
    QStringList folders = FileIndexerConfig::self()->includeFolders();
    Q_FOREACH (const QString& folder, folders) {
        if (!folder.startsWith(home)) {
            watchFolder(folder);
        }
    }
#else
    connectToKDirNotify();
#endif

    // we automatically watch newly mounted media - it is very unlikely that anything non-interesting is mounted
    /*
    m_removableMediaCache = new RemovableMediaCache(this);
    connect(m_removableMediaCache, SIGNAL(deviceMounted(const Baloo::RemovableMediaCache::Entry*)),
            this, SLOT(slotDeviceMounted(const Baloo::RemovableMediaCache::Entry*)));
    connect(m_removableMediaCache, SIGNAL(deviceTeardownRequested(const Baloo::RemovableMediaCache::Entry*)),
            this, SLOT(slotDeviceTeardownRequested(const Baloo::RemovableMediaCache::Entry*)));
    addWatchesForMountedRemovableMedia();
    */

    connect(FileIndexerConfig::self(), SIGNAL(configChanged()),
            this, SLOT(updateIndexedFoldersWatches()));
}


FileWatch::~FileWatch()
{
}


// FIXME: listen to Create for folders!
void FileWatch::watchFolder(const QString& path)
{
    kDebug() << path;
#ifdef BUILD_KINOTIFY
    if (m_dirWatch && !m_dirWatch->watchingPath(path))
        m_dirWatch->addWatch(path,
                             KInotify::WatchEvents(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf | KInotify::EventCloseWrite | KInotify::EventCreate),
                             KInotify::WatchFlags());
#endif
}

void FileWatch::slotFileMoved(const QString& urlFrom, const QString& urlTo)
{
    if (!ignorePath(urlFrom) || !ignorePath(urlTo)) {
        m_metadataMover->moveFileMetadata(urlFrom, urlTo);
    }
}


void FileWatch::slotFilesDeleted(const QStringList& paths)
{
    QStringList urls;
    Q_FOREACH (const QString& path, paths) {
        if (!ignorePath(path)) {
            urls << path;
        }
    }

    if (!urls.isEmpty()) {
        m_metadataMover->removeFileMetadata(urls);
    }
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

    // If we have recieved a FileClosedAfterWrite event, then the file must have been
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
                                          this, SIGNAL(slotFileMoved(const QString&, const QString&)));
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FilesRemoved",
                                          this, SIGNAL(slotFilesDeleted(const QStringList&)));
}


#ifdef BUILD_KINOTIFY

#include <syslog.h>
#include <kauth.h>

//Try to raise the inotify watch limit by executing
//a helper which modifies /proc/sys/fs/inotify/max_user_watches
bool raiseWatchLimit()
{
    KAuth::Action limitAction("org.kde.nepomuk.filewatch.raiselimit");
    limitAction.setHelperID("org.kde.nepomuk.filewatch");

    KAuth::ActionReply reply = limitAction.execute();
    if (reply.failed()) {
        return false;
    }
    return true;
}

//This slot is connected to a signal emitted in KInotify when
//inotify_add_watch fails with ENOSPC.
void FileWatch::slotInotifyWatchUserLimitReached(const QString& path)
{
    if (raiseWatchLimit()) {
        kDebug() << "Successfully raised watch limit, re-adding " << path;
        if (m_dirWatch)
            m_dirWatch->resetUserLimit();
        watchFolder(path);
    } else {
        //If we got here, we hit the limit and couldn't authenticate to raise it,
        // so put something in the syslog so someone notices.
        syslog(LOG_USER | LOG_WARNING, "KDE Nepomuk Filewatch service reached the folder watch limit. File changes may be ignored.");
        // we do it the brutal way for now hoping with new kernels and defaults this will never happen
        // Delete the KInotify and switch to KDirNotify dbus signals
        if (m_dirWatch) {
            m_dirWatch->deleteLater();
            m_dirWatch = 0;
        }
        connectToKDirNotify();
    }
}
#endif


bool FileWatch::ignorePath(const QString& path)
{
    // when using KInotify there is no need to check the folder since
    // we only watch interesting folders to begin with.
    return m_pathExcludeRegExpCache->filenameMatch(path);
}


void FileWatch::updateIndexedFoldersWatches()
{
#ifdef BUILD_KINOTIFY
    if (m_dirWatch) {
        QStringList folders = FileIndexerConfig::self()->includeFolders();
        Q_FOREACH (const QString& folder, folders) {
            m_dirWatch->removeWatch(folder);
            watchFolder(folder);
        }
    }
#endif
}


/*
void FileWatch::addWatchesForMountedRemovableMedia()
{
    Q_FOREACH(const RemovableMediaCache::Entry * entry, m_removableMediaCache->allMedia()) {
        if (entry->isMounted())
            slotDeviceMounted(entry);
    }
}

void FileWatch::slotDeviceMounted(const Baloo::RemovableMediaCache::Entry* entry)
{
    //
    // tell the file indexer to update the newly mounted device
    //
    KConfig fileIndexerConfig("nepomukstrigirc");
    const QByteArray devGroupName("Device-" + entry->url().toUtf8());
    int index = 0;
    // Old-format -> port to new format
    if (fileIndexerConfig.group("Devices").hasKey(entry->url())) {
        KConfigGroup devicesGroup = fileIndexerConfig.group("Devices");
        index = devicesGroup.readEntry(entry->url(), false) ? 1 : -1;
        devicesGroup.deleteEntry(entry->url());

        KConfigGroup devGroup = fileIndexerConfig.group(devGroupName);
        devGroup.writeEntry("mount path", entry->mountPath());
        if (index == 1)
            devGroup.writeEntry("folders", QStringList() << QLatin1String("/"));
        else if (index == -1)
            devGroup.writeEntry("exclude folders", QStringList() << QLatin1String("/"));

        fileIndexerConfig.sync();

        // Migrate the existing filex data
        RemovableMediaDataMigrator* migrator = new RemovableMediaDataMigrator(entry);
        QThreadPool::globalInstance()->start(migrator);
    }

    // Already exists
    if (fileIndexerConfig.hasGroup(devGroupName)) {
        // Inform the indexer to start the indexing process

        org::kde::nepomuk::FileIndexer fileIndexer("org.kde.nepomuk.services.nepomukfileindexer", "/nepomukfileindexer", QDBusConnection::sessionBus());
        if (fileIndexer.isValid()) {
            KConfigGroup group = fileIndexerConfig.group(devGroupName);
            const QString mountPath = group.readEntry("mount path", QString());
            if (!mountPath.isEmpty()) {
                const QStringList folders = group.readPathEntry("folders", QStringList());
                Q_FOREACH (const QString & folder, folders) {
                    QString path = mountPath + folder;
                    fileIndexer.indexFolder(path, true, false);
                }
            }
        }

        index = 1;
    }

    const bool indexNewlyMounted = fileIndexerConfig.group("RemovableMedia").readEntry("index newly mounted", false);
    const bool askIndividually = fileIndexerConfig.group("RemovableMedia").readEntry("ask user", false);

    if (index == 0 && indexNewlyMounted && !askIndividually) {
        KConfigGroup deviceGroup = fileIndexerConfig.group("Device-" + entry->url());
        deviceGroup.writeEntry("folders", QStringList() << entry->mountPath());

        index = 1;
    }

    // ask the user if we should index
    else if (index == 0 && indexNewlyMounted && askIndividually) {
        kDebug() << "Device unknown. Asking user for action.";
        (new RemovableDeviceIndexNotification(entry, this))->sendEvent();
    }

    //
    // Install the watches
    //
    KConfig config("nepomukstrigirc");
    KConfigGroup cfg = config.group("RemovableMedia");

    if (cfg.readEntry<bool>("add watches", true)) {
        QString path = entry->mountPath();
        // If the device is not a storage device, mountPath returns QString().
        // In this case do not try to install watches.
        if (path.isEmpty())
            return;
        if (entry->device().isDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
            if (cfg.readEntry<bool>("add watches network share", false)) {
                kDebug() << "Installing watch for network share at mount point" << path;
                watchFolder(path);
            }
        } else {
            kDebug() << "Installing watch for removable storage at mount point" << path;
            // vHanda: Perhaps this should only be done if we have some metadata on the removable media
            // and if we do not then we add the watches when we get some metadata?
            watchFolder(path);
        }
    }
}

void FileWatch::slotDeviceTeardownRequested(const Baloo::RemovableMediaCache::Entry* entry)
{
#ifdef BUILD_KINOTIFY
    if (m_dirWatch) {
        kDebug() << entry->mountPath();
        m_dirWatch->removeWatch(entry->mountPath());
    }
#endif
}

*/

void FileWatch::slotActiveFileQueueTimeout(const QString& url)
{
    kDebug() << url;
    Q_EMIT indexFile(url);
}

void FileWatch::slotMovedWithoutData(const QString& url)
{
    Q_EMIT indexFile(url);
}

