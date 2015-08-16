/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BALOO_FILEINDEXSCHEDULER_H
#define BALOO_FILEINDEXSCHEDULER_H

#include <QObject>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>

#include "filecontentindexerprovider.h"
#include "eventmonitor.h"
#include "indexerstate.h"

namespace Baloo {

class Database;
class FileIndexerConfig;
class FileContentIndexer;

class FileIndexScheduler : public QObject
{
    Q_OBJECT
public:
    FileIndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent = 0);

Q_SIGNALS:
    void stateChanged(IndexerState state);
    void indexingFile(QString filePath);

public Q_SLOTS:
    void indexNewFile(const QString& file) {
        if (!m_newFiles.contains(file)) {
            m_newFiles << file;
            QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
        }
    }

    void indexModifiedFile(const QString& file) {
        if (!m_modifiedFiles.contains(file)) {
            m_modifiedFiles << file;
            QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
        }
    }

    void indexXAttrFile(const QString& file) {
        if (!m_xattrFiles.contains(file)) {
            m_xattrFiles << file;
            QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
        }
    }

    void handleFileRemoved(const QString& file);

    void scheduleIndexing();

    void setSuspend(bool suspend);
    uint getRemainingTime();

    IndexerState state() const { return m_indexerState; }

private Q_SLOTS:
    void powerManagementStatusChanged(bool isOnBattery);
    void idleStatusChanged(bool isIdle);

private:
    Database* m_db;
    FileIndexerConfig* m_config;

    QStringList m_newFiles;
    QStringList m_modifiedFiles;
    QStringList m_xattrFiles;

    QThreadPool m_threadPool;

    FileContentIndexerProvider m_provider;
    FileContentIndexer* m_contentIndexer;

    EventMonitor* m_eventMonitor;

    IndexerState m_indexerState;
};

}

#endif // BALOO_FILEINDEXSCHEDULER_H
