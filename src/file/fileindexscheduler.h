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
#include "powerstatemonitor.h"
#include "indexerstate.h"
#include "timeestimator.h"

namespace Baloo {

class Database;
class FileIndexerConfig;
class FileContentIndexer;

class FileIndexScheduler : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.scheduler")

    Q_PROPERTY(int state READ state NOTIFY stateChanged)
public:
    FileIndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent = 0);
    ~FileIndexScheduler() Q_DECL_OVERRIDE;
    int state() const { return m_indexerState; }

Q_SIGNALS:
    Q_SCRIPTABLE void stateChanged(int state);

public Q_SLOTS:
    void indexNewFile(const QString& file) {
        if (!m_newFiles.contains(file)) {
            m_newFiles << file;
            QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
        }
    }

    void indexModifiedFile(const QString& file) {
        if (!m_modifiedFiles.contains(file)) {
            m_modifiedFiles << file;
            QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
        }
    }

    void indexXAttrFile(const QString& file) {
        if (!m_xattrFiles.contains(file)) {
            m_xattrFiles << file;
            QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
        }
    }

    void handleFileRemoved(const QString& file);

    void scheduleIndexing();

    Q_SCRIPTABLE void suspend() { setSuspend(true); }
    Q_SCRIPTABLE void resume() { setSuspend(false); }
    Q_SCRIPTABLE uint getRemainingTime();
    Q_SCRIPTABLE void checkUnindexedFiles();

private Q_SLOTS:
    void powerManagementStatusChanged(bool isOnBattery);

private:
    void setSuspend(bool suspend);

    Database* m_db;
    FileIndexerConfig* m_config;

    QStringList m_newFiles;
    QStringList m_modifiedFiles;
    QStringList m_xattrFiles;

    QThreadPool m_threadPool;

    FileContentIndexerProvider m_provider;
    FileContentIndexer* m_contentIndexer;

    PowerStateMonitor m_powerMonitor;

    IndexerState m_indexerState;
    TimeEstimator m_timeEstimator;

    bool m_checkUnindexedFiles;
};

}

#endif // BALOO_FILEINDEXSCHEDULER_H
