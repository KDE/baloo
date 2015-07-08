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

#include "fileindexscheduler.h"

#include "firstrunindexer.h"
#include "newfileindexer.h"
#include "modifiedfileindexer.h"
#include "xattrindexer.h"
#include "filecontentindexer.h"
#include "filecontentindexerprovider.h"

#include "fileindexerconfig.h"
#include "timeestimator.h"

#include <QTimer>
#include <QDebug>

using namespace Baloo;

FileIndexScheduler::FileIndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
    , m_provider(db)
    , m_contentIndexer(0)
    , m_indexerState(Idle)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    m_threadPool.setMaxThreadCount(1);

    m_eventMonitor = new EventMonitor(this);
    m_eventMonitor->enable();
    connect(m_eventMonitor, &EventMonitor::powerManagementStatusChanged,
            this, &FileIndexScheduler::powerManagementStatusChanged);
}

void FileIndexScheduler::scheduleIndexing()
{
    if (m_threadPool.activeThreadCount() || m_indexerState == Suspended) {
        return;
    }
    qDebug() << "SCHEDULE";

    m_contentIndexerRunning = false;

    if (m_config->isInitialRun()) {
        qDebug() << m_config->includeFolders();
        auto runnable = new FirstRunIndexer(m_db, m_config, m_config->includeFolders());
        connect(runnable, &FirstRunIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_indexerState = FirstRun;
        return;
    }

    if (!m_newFiles.isEmpty()) {
        qDebug() << "NEW" << m_newFiles;
        auto runnable = new NewFileIndexer(m_db, m_config, m_newFiles);
        connect(runnable, &NewFileIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_newFiles.clear();
        m_indexerState = NewFiles;
        return;
    }

    if (!m_modifiedFiles.isEmpty()) {
        qDebug() << "MOD" << m_modifiedFiles;
        auto runnable = new ModifiedFileIndexer(m_db, m_config, m_modifiedFiles);
        connect(runnable, &ModifiedFileIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_modifiedFiles.clear();
        m_indexerState = ModifiedFiles;
        return;
    }

    if (!m_xattrFiles.isEmpty()) {
        qDebug() << "XATTR" << m_xattrFiles;
        auto runnable = new XAttrIndexer(m_db, m_config, m_xattrFiles);
        connect(runnable, &XAttrIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_xattrFiles.clear();
        m_indexerState = XAttrFiles;
        return;
    }

    if (m_provider.size() && !m_eventMonitor->isOnBattery()) {
        qDebug() << "Content: " << m_provider.size();
        m_contentIndexerRunning = true;
        if (m_contentIndexer == 0) {
            m_contentIndexer = new FileContentIndexer(&m_provider);
        }
        connect(m_contentIndexer, &FileContentIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(m_contentIndexer);
        m_indexerState = ContentIndexing;
        return;
    }
    m_indexerState = Idle;
    qDebug() << "IDLE";
}

static void removeStartsWith(QStringList& list, const QString& dir)
{
    QMutableListIterator<QString> it(list);
    while (it.hasNext()) {
        QString file = it.next();
        if (file.startsWith(dir)) {
            it.remove();
        }
    }
}

void FileIndexScheduler::handleFileRemoved(const QString& file)
{
    if (!file.endsWith('/')) {
        m_newFiles.removeOne(file);
        m_modifiedFiles.removeOne(file);
        m_xattrFiles.removeOne(file);
    }
    else {
        removeStartsWith(m_newFiles, file);
        removeStartsWith(m_modifiedFiles, file);
        removeStartsWith(m_xattrFiles, file);
    }
}

void FileIndexScheduler::powerManagementStatusChanged(bool isOnBattery)
{
    qDebug() << "Power state changed";
    if (isOnBattery && m_indexerState == ContentIndexing) {
        qDebug() << "On battery stopping content indexer";
        m_contentIndexer->quit();
        m_contentIndexer = 0;
        //TODO: Maybe we can add a special state for suspended due to being on battery.
        m_indexerState = Idle;
    } else if (!isOnBattery) {
        QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
    }
}

void FileIndexScheduler::setSuspend(bool suspend)
{
    if (suspend) {
        qDebug() << "Suspending";
        m_eventMonitor->disable();
        if (m_indexerState == ContentIndexing) {
            m_contentIndexer->quit();
            m_contentIndexer = 0;
        }
        m_indexerState = Suspended;
    } else {
        qDebug() << "Resuming";
        m_eventMonitor->enable();
        m_indexerState = Idle;
        scheduleIndexing();
    }
}

uint FileIndexScheduler::getRemainingTime()
{
    if (m_indexerState != ContentIndexing) {
        return 0;
    }
    TimeEstimator estimator;
    estimator.setFilesLeft(m_provider.size());
    estimator.setBatchTimings(m_contentIndexer->batchTimings());
    return estimator.calculateTimeLeft();
}
