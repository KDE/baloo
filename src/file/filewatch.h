/* This file is part of the KDE Project
   Copyright (c) 2007-2011 Sebastian Trueg <trueg@kde.org>

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

#ifndef _FILE_WATCH_H_
#define _FILE_WATCH_H_

#include <QObject>
#include <QtCore/QVariant>
#include <QtCore/QSet>

#include "removablemediacache.h"

class KInotify;
class KUrl;
class RegExpCache;
class ActiveFileQueue;
class QThread;
class Database;

namespace Baloo
{

class MetadataMover;

class FileWatch : public QObject
{
    Q_OBJECT

public:
    FileWatch(Database* db, QObject* parent = 0);
    ~FileWatch();

    /**
     * Tells the file indexer to update the file (it can also be a folder but
     * then updating will not be recursive) at \p path.
     */
    static void updateFileViaFileIndexer(const QString& path);

    /**
     * Tells the file indexer to update the folder at \p path or the folder
     * containing \p path in case it is a file.
     */
    static void updateFolderViaFileIndexer(const QString& path);

Q_SIGNALS:
    /**
     * Emitted each time the status/activity of the FileWatcher changes
     *
     * @p status what the watcher is doing
     *    @arg 0 idle
     *    @arg 1 working
     *
     * @p msg translated status message that indicates what is happening
     **/
    Q_SCRIPTABLE void status(int status, QString msg);

    /**
     * Emitted when the watcher starts to do something
     */
    Q_SCRIPTABLE void metadataUpdateStarted();

    /**
     * Emitted when the watcher stops to do something
     */
    Q_SCRIPTABLE void metadataUpdateStopped();

public Q_SLOTS:
    /**
     * Returns if the watcher is doing something
     *
     * @return @arg true watcher is working
     *         @arg false watcher is idle
     */
    Q_SCRIPTABLE bool isUpdatingMetaData() const;

    /**
     * Returns a translated status message that indicates what is happening
     *
     * @return translated status string
     **/
    Q_SCRIPTABLE QString statusMessage() const;

private Q_SLOTS:
    void slotFileMoved(const QString& from, const QString& to);
    void slotFileDeleted(const QString& urlString, bool isDir);
    void slotFilesDeleted(const QStringList& path);
    void slotFileCreated(const QString& path, bool isDir);
    void slotFileClosedAfterWrite(const QString&);
    void slotMovedWithoutData(const QString&);
    void connectToKDirNotify();
#ifdef BUILD_KINOTIFY
    void slotInotifyWatchUserLimitReached(const QString&);
#endif
    /**
     * To be called whenever the list of indexed folders changes. This is done because
     * the indexed folders are watched with the 'KInotify::EventCreate' event, and the
     * non-indexed folders are not.
     */
    void updateIndexedFoldersWatches();

    /**
     * Connected to each removable media. Adds a watch for the mount point,
     * cleans up the index with respect to removed files, and optionally
     * tells the indexer service to run on the mount path.
     */
    void slotDeviceMounted(const Baloo::RemovableMediaCache::Entry*);

    /**
     * Connected to each removable media.
     * Removes all the watches that were added for that removable media
     */
    void slotDeviceTeardownRequested(const Baloo::RemovableMediaCache::Entry*);

    void slotActiveFileQueueTimeout(const QString& url);

    /**
     * @brief When called via MetaMover (signal) the state of the watcher will be active and the status message will be set
     *
     * @param newStatus new translated status string telling what the current task is
     */
    void updateStatusMessage(const QString& newStatus);

    /**
     * @brief When called via MetaMover (signal) the state of the watcher is idle
     *
     * Sets the status message to indicate that the watcher is idle.
     */
    void resetStatusMessage();

private:
    /** Watch a folder, provided it is not already watched*/
    void watchFolder(const QString& path);

    /**
     * Adds watches for all mounted removable media.
     */
    void addWatchesForMountedRemovableMedia();

    /**
     * Returns true if the path is one that should be always ignored.
     * This includes such things like temporary files and folders as
     * they are created for example by build systems.
     */
    bool ignorePath(const QString& path);

    Database* m_db;

    QThread* m_metadataMoverThread;
    MetadataMover* m_metadataMover;

#ifdef BUILD_KINOTIFY
    KInotify* m_dirWatch;
#endif

    RegExpCache* m_pathExcludeRegExpCache;
    RemovableMediaCache* m_removableMediaCache;

    /// queue used to "compress" constant file modifications like downloads
    ActiveFileQueue* m_fileModificationQueue;

    bool m_isIdle;              /**< Current state of the watcher
                                       * @arg true is idle
                                       * @arg false is doing something
                                       */
    QString m_statusMessage;    /**< temporary saved status message telling what is going on */
};
}

#endif
