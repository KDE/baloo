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
class RegExpCache;
class ActiveFileQueue;
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

Q_SIGNALS:
    void indexFile(const QString& string);
    void fileRemoved(int id);

    void installedWatches();

private Q_SLOTS:
    void slotFileMoved(const QString& from, const QString& to);
    void slotFileDeleted(const QString& urlString, bool isDir);
    void slotFilesDeleted(const QStringList& path);
    void slotFileCreated(const QString& path, bool isDir);
    void slotFileClosedAfterWrite(const QString&);
    void connectToKDirNotify();
#ifdef BUILD_KINOTIFY
    void slotInotifyWatchUserLimitReached(const QString&);
#endif
    void slotMovedWithoutData(const QString& url);

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
    //void slotDeviceMounted(const Baloo::RemovableMediaCache::Entry*);

    /**
     * Connected to each removable media.
     * Removes all the watches that were added for that removable media
     */
    //void slotDeviceTeardownRequested(const Baloo::RemovableMediaCache::Entry*);

    void slotActiveFileQueueTimeout(const QString& url);

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

    MetadataMover* m_metadataMover;

#ifdef BUILD_KINOTIFY
    KInotify* m_dirWatch;
#endif

    RegExpCache* m_pathExcludeRegExpCache;
    //RemovableMediaCache* m_removableMediaCache;

    /// queue used to "compress" constant file modifications like downloads
    ActiveFileQueue* m_fileModificationQueue;
};
}

#endif
