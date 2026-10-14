/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mainhub.h"
#include "fileindexerconfig.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QTimer>

using namespace Baloo;
using namespace Qt::StringLiterals;

static const auto IDLE_MONITOR_SERVICE{u"org.kde.baloo_idle_monitor"_s};

MainHub::MainHub(Database *db, FileIndexerConfig *config, bool firstRun)
    : m_config(config)
    , m_fileWatcher(db, config, this)
    , m_fileIndexScheduler(db, config, firstRun, this)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    connect(&m_fileWatcher, &FileWatch::indexNewFile, &m_fileIndexScheduler, &FileIndexScheduler::indexNewFile);
    connect(&m_fileWatcher, &FileWatch::indexModifiedFile, &m_fileIndexScheduler, &FileIndexScheduler::indexModifiedFile);
    connect(&m_fileWatcher, &FileWatch::indexXAttr, &m_fileIndexScheduler, &FileIndexScheduler::indexXAttrFile);
    connect(&m_fileWatcher, &FileWatch::fileRemoved, &m_fileIndexScheduler, &FileIndexScheduler::handleFileRemoved);

    connect(&m_fileWatcher, &FileWatch::installedWatches, &m_fileIndexScheduler, &FileIndexScheduler::scheduleIndexing);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots |
                        QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportAdaptors);

    QTimer::singleShot(5000, this, [this] {
        // Delay expensive housekeeping and content indexing
        // so we don't end up consuming excessive resources on login
        m_fileIndexScheduler.startupFinished();
    });

    // Try to start the idle monitor
    auto monitorPing{QDBusMessage::createMethodCall(IDLE_MONITOR_SERVICE, u"/"_s, u"org.freedesktop.DBus.Peer"_s, u"Ping"_s)};
    bus.asyncCall(monitorPing);

    QTimer::singleShot(0, &m_fileWatcher, &FileWatch::updateIndexedFoldersWatches);
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

#include "moc_mainhub.cpp"
