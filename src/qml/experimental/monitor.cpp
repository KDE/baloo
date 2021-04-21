/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "monitor.h"

#include "database.h"
#include "transaction.h"
#include "global.h"
#include "config.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QDBusServiceWatcher>
#include <QProcess>

#include <KFormat>

using namespace Baloo;
Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_bus(QDBusConnection::sessionBus())
    , m_filePath(QStringLiteral("Idle"))
    , m_scheduler(nullptr)
    , m_fileindexer(nullptr)
    , m_remainingTime(QStringLiteral("Estimating"))
{
    m_scheduler = new org::kde::baloo::scheduler(QStringLiteral("org.kde.baloo"),
                                          QStringLiteral("/scheduler"),
                                          m_bus, this);

    m_fileindexer = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
                                                    QStringLiteral("/fileindexer"),
                                                    m_bus, this);

    connect(m_fileindexer, &org::kde::baloo::fileindexer::startedIndexingFile,
            this, &Monitor::newFile);

    connect(m_scheduler, &org::kde::baloo::scheduler::stateChanged,
            this, &Monitor::slotIndexerStateChanged);

    QDBusServiceWatcher* balooWatcher = new QDBusServiceWatcher(m_scheduler->service(),
                                                            m_bus,
                                                            QDBusServiceWatcher::WatchForOwnerChange,
                                                            this);
    connect(balooWatcher, &QDBusServiceWatcher::serviceRegistered, this, &Monitor::balooStarted);
    connect(balooWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this]() {
        m_balooRunning = false;
        m_indexerState = Baloo::Unavailable;
        Q_EMIT balooStateChanged();
        Q_EMIT indexerStateChanged();
    });

    if (m_scheduler->isValid()) {
        // baloo is already running
        balooStarted();
    }
}

void Monitor::newFile(const QString& filePath)
{
    m_filePath = filePath;
    if (m_totalFiles == 0) {
        fetchTotalFiles();
    }
    ++m_filesIndexed;
    Q_EMIT newFileIndexed();

    auto now = QDeadlineTimer::current();
    if (now > m_remainingTimeTimer) {
        updateRemainingTime();
        m_remainingTimeTimer = now + 1000;
    }
}

QString Monitor::suspendState() const
{
    return m_indexerState == Baloo::Suspended ?  QStringLiteral("Resume") : QStringLiteral("Suspend");
}

void Monitor::toggleSuspendState()
{
    if (m_indexerState == Baloo::Suspended) {
        m_scheduler->resume();
    } else {
        m_scheduler->suspend();
    }
}

void Monitor::balooStarted()
{
    m_balooRunning = true;
    m_fileindexer->registerMonitor();

    slotIndexerStateChanged(m_scheduler->state());
    Q_EMIT balooStateChanged();
}

void Monitor::fetchTotalFiles()
{
    Baloo::Database *db = Baloo::globalDatabaseInstance();
    if (db->open(Baloo::Database::ReadOnlyDatabase)) {
        Baloo::Transaction tr(db, Baloo::Transaction::ReadOnly);
        m_totalFiles = tr.size();
        m_filesIndexed = tr.size() - tr.phaseOneSize();
        Q_EMIT totalFilesChanged();
        Q_EMIT newFileIndexed();
    }
}

void Monitor::startBaloo()
{
    const QString exe = QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR "/baloo_file");
    QProcess::startDetached(exe, QStringList());
}

void Monitor::updateRemainingTime()
{
    auto remainingTime = m_scheduler->getRemainingTime();
    if ((remainingTime != m_remainingTimeSeconds) && (remainingTime > 0)) {
        m_remainingTime = KFormat().formatSpelloutDuration(remainingTime);
        m_remainingTimeSeconds = remainingTime;
        Q_EMIT remainingTimeChanged();
    }
}

void Monitor::slotIndexerStateChanged(int state)
{
    Baloo::IndexerState newState = static_cast<Baloo::IndexerState>(state);

    if (m_indexerState != newState) {
        m_indexerState = newState;
        fetchTotalFiles();
        if (m_indexerState != Baloo::ContentIndexing) {
            m_filePath = QString();
        }
        Q_EMIT indexerStateChanged();
    }
}

