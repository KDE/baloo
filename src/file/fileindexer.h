/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2013 Vishesh Handa <handa.vish@gmail.com>

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

#ifndef BALOO_FILEINDEXER_SERVICE_H_
#define BALOO_FILEINDEXER_SERVICE_H_

#include <QTimer>

namespace Baloo
{

class Database;
class IndexScheduler;
class FileIndexerConfig;

/**
 * Service controlling the file indexer
 */
class FileIndexer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo")

public:
    FileIndexer(Database* db, FileIndexerConfig* config, QObject* parent = 0);
    ~FileIndexer();

Q_SIGNALS:
    Q_SCRIPTABLE void statusChanged();
    Q_SCRIPTABLE void indexingStarted();
    Q_SCRIPTABLE void indexingStopped();
    Q_SCRIPTABLE void fileIndexingDone();
    void configChanged();

public Q_SLOTS:
    /**
     * Checks if the file system needs to be scanned and updates the folders
     * accordingly
     */
    void update();

    /**
     * Quits the application. This may not be the best place to keep such
     * a slot, but it's simpler than creating a new interface for now
     */
    Q_SCRIPTABLE void quit() const;

    /**
     * @brief Translated status message of the current Indexer behaviour.
     */
    Q_SCRIPTABLE QString statusMessage() const;

    Q_SCRIPTABLE bool isSuspended() const;
    Q_SCRIPTABLE bool isIndexing() const;

    Q_SCRIPTABLE void resume() const;
    Q_SCRIPTABLE void suspend() const;
    Q_SCRIPTABLE void setSuspended(bool);

    /**
     * Update all folders configured to be indexed.
     */
    Q_SCRIPTABLE void updateAllFolders(bool forced);

    /**
     * Index a folder independent of its configuration status.
     */
    Q_SCRIPTABLE void indexFolder(const QString& path, bool forced);

    /**
     * Index a specific file
     */
    Q_SCRIPTABLE void indexFile(const QString& path);

    /**
     * Index only the extended attributes of the file
     */
    Q_SCRIPTABLE void indexXAttr(const QString& path);

    Q_SCRIPTABLE void updateConfig();

private Q_SLOTS:
    void slotBasicIndexingDone();

private:
    IndexScheduler* m_indexScheduler;
    FileIndexerConfig* m_config;
    bool m_startupUpdateDone;
    bool m_initalRun;
};
}

#endif
