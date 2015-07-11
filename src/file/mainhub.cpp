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

#include "mainhub.h"
#include "fileindexerconfig.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QTimer>
#include <QDBusMetaType>

using namespace Baloo;

MainHub::MainHub(Database* db, FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
    , m_fileWatcher(db, config, this)
    , m_fileIndexScheduler(db, config, this)
    , m_isSuspended(false)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    qRegisterMetaType<IndexerState>("IndexerState");
    qDBusRegisterMetaType<IndexerState>();

    connect(&m_fileWatcher, &FileWatch::indexNewFile, &m_fileIndexScheduler, &FileIndexScheduler::indexNewFile);
    connect(&m_fileWatcher, &FileWatch::indexModifiedFile, &m_fileIndexScheduler, &FileIndexScheduler::indexModifiedFile);
    connect(&m_fileWatcher, &FileWatch::indexXAttr, &m_fileIndexScheduler, &FileIndexScheduler::indexXAttrFile);
    connect(&m_fileWatcher, &FileWatch::fileRemoved, &m_fileIndexScheduler, &FileIndexScheduler::handleFileRemoved);

    connect(&m_fileWatcher, &FileWatch::installedWatches, &m_fileIndexScheduler, &FileIndexScheduler::scheduleIndexing);
    connect(&m_fileIndexScheduler, &FileIndexScheduler::stateChanged, this, &MainHub::stateChanged);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/indexer"), this, QDBusConnection::ExportAllSlots |
                        QDBusConnection::ExportScriptableSignals);

    QTimer::singleShot(0, &m_fileWatcher, SLOT(watchIndexedFolders()));
}

void MainHub::quit() const
{
    QCoreApplication::instance()->quit();
}

void MainHub::updateConfig()
{
    m_config->forceConfigUpdate();
    // FIXME!!
    //m_fileIndexer.updateConfig();
    m_fileWatcher.updateIndexedFoldersWatches();
}

void MainHub::resume()
{
    m_fileIndexScheduler.setSuspend(false);
    m_isSuspended = false;
}

void MainHub::suspend()
{
    m_fileIndexScheduler.setSuspend(true);
    m_isSuspended = true;
}

uint MainHub::getRemainingTime()
{
    return m_fileIndexScheduler.getRemainingTime();
}

bool MainHub::isSuspended() const
{
    return m_isSuspended;
}

IndexerState MainHub::state() const
{
    return m_fileIndexScheduler.state();
}
