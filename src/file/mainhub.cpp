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
#include "mainadaptor.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QTimer>

using namespace Baloo;

MainHub::MainHub(Database* db, FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
    , m_fileWatcher(db, config, this)
    , m_fileIndexScheduler(db, config, this)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    connect(&m_fileWatcher, &FileWatch::indexNewFile, &m_fileIndexScheduler, &FileIndexScheduler::indexNewFile);
    connect(&m_fileWatcher, &FileWatch::indexModifiedFile, &m_fileIndexScheduler, &FileIndexScheduler::indexModifiedFile);
    connect(&m_fileWatcher, &FileWatch::indexXAttr, &m_fileIndexScheduler, &FileIndexScheduler::indexXAttrFile);
    connect(&m_fileWatcher, &FileWatch::fileRemoved, &m_fileIndexScheduler, &FileIndexScheduler::handleFileRemoved);

    connect(&m_fileWatcher, &FileWatch::installedWatches, &m_fileIndexScheduler, &FileIndexScheduler::scheduleIndexing);

    MainAdaptor* main = new MainAdaptor(this);
    Q_UNUSED(main)

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots |
                        QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportAdaptors);

    if (!m_config->isInitialRun()) {
        // Delay these checks so we don't end up consuming excessive resources on login
        QTimer::singleShot(5000, this, [this] {
            m_fileIndexScheduler.checkUnindexedFiles();
            m_fileIndexScheduler.checkStaleIndexEntries();
        });
    }
    QTimer::singleShot(0, &m_fileWatcher, &FileWatch::watchIndexedFolders);
}

void MainHub::quit() const
{
    QCoreApplication::instance()->quit();
}

void MainHub::updateConfig()
{
    m_config->forceConfigUpdate();
    m_fileWatcher.updateIndexedFoldersWatches();
    m_fileIndexScheduler.updateConfig();
}

void MainHub::registerBalooWatcher(const QString &service)
{
    m_fileWatcher.registerBalooWatcher(service);
}
