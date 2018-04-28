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

#ifndef BALOO_FILE_WATCH_H_
#define BALOO_FILE_WATCH_H_

#include <QObject>
#include "pendingfile.h"

class KInotify;

namespace Baloo
{
class Database;
class MetadataMover;
class FileIndexerConfig;
class PendingFileQueue;
class FileWatchTest;
class MainHub;

class FileWatch : public QObject
{
    Q_OBJECT

public:
    FileWatch(Database* db, FileIndexerConfig* config, Baloo::MainHub *dbusInterface, QObject* parent = nullptr);
    ~FileWatch();

public Q_SLOTS:
    /**
     * To be called whenever the list of indexed folders changes. This is done because
     * the indexed folders are watched with the 'KInotify::EventCreate' event, and the
     * non-indexed folders are not.
     */
    void updateIndexedFoldersWatches();

    void watchIndexedFolders();

    void registerBalooWatcher(const QString &service);

Q_SIGNALS:
    void indexNewFile(const QString& string);
    void indexModifiedFile(const QString& string);
    void indexXAttr(const QString& path);

    /**
     * This signal is emitted when a file has been removed, and everyone else
     * should update their caches
     */
    void fileRemoved(const QString& path);

    void installedWatches();

private Q_SLOTS:
    void slotFileMoved(const QString& from, const QString& to);
    void slotFileDeleted(const QString& urlString, bool isDir);
    void slotFileCreated(const QString& path, bool isDir);
    void slotFileClosedAfterWrite(const QString&);
    void slotAttributeChanged(const QString& path);
    void slotFileModified(const QString& path);
    void slotInotifyWatchUserLimitReached(const QString&);

private:
    /** Watch a folder, provided it is not already watched*/
    void watchFolder(const QString& path);

    Database* m_db;

    MetadataMover* m_metadataMover;
    FileIndexerConfig* m_config;

    KInotify* m_dirWatch;

    /// queue used to "compress" multiple file events like downloads
    PendingFileQueue* m_pendingFileQueue;

    friend class FileWatchTest;
};
}

#endif
