/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "monitor.h"

#include "database.h"
#include "transaction.h"
#include "global.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QStandardPaths>
#include <QDBusServiceWatcher>
#include <QProcess>

#include <KFormat>

using namespace Baloo;
Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_bus(QDBusConnection::sessionBus())
    , m_filePath(QStringLiteral("Idle"))
    , m_scheduler(0)
    , m_fileindexer(0)
    , m_filesIndexed(0)
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

    if (m_scheduler->isValid()) {
        // baloo is already running
        balooStarted(m_scheduler->service());

    } else {
        m_balooRunning = false;
        QDBusServiceWatcher* balooWatcher = new QDBusServiceWatcher(m_scheduler->service(),
                                                                m_bus,
                                                                QDBusServiceWatcher::WatchForRegistration,
                                                                this);
        connect(balooWatcher, &QDBusServiceWatcher::serviceRegistered, this, &Monitor::balooStarted);
    }
}

void Monitor::newFile(const QString& filePath)
{
    if (m_totalFiles == 0) {
        fetchTotalFiles();
    }
    m_filePath = filePath;
    if (++m_filesIndexed == m_totalFiles) {
        m_filePath = QStringLiteral("Done");
    }
    Q_EMIT newFileIndexed();

    if (m_filesIndexed % (m_scheduler->getBatchSize() * 5) == 0) {
        updateRemainingTime();
    }
}

QString Monitor::suspendState() const
{
    return m_indexerState == Baloo::Suspended ?  QStringLiteral("Resume") : QStringLiteral("Suspend");
}

void Monitor::toggleSuspendState()
{
    Q_ASSERT(m_scheduler != 0);

    if (m_indexerState == Baloo::Suspended) {
        m_scheduler->resume();
    } else {
        m_scheduler->suspend();
    }
}

void Monitor::balooStarted(const QString& service)
{
    Q_ASSERT(service == QStringLiteral("org.kde.baloo"));

    m_balooRunning = true;
    m_fileindexer->registerMonitor();

    slotIndexerStateChanged(m_scheduler->state());
    qDebug() << "fetched suspend state";
    fetchTotalFiles();
    if (m_indexerState == Baloo::ContentIndexing) {
        m_filePath = m_fileindexer->currentFile();
        updateRemainingTime();
    }
    Q_EMIT balooStateChanged();
}

void Monitor::fetchTotalFiles()
{
    Baloo::Database *db = Baloo::globalDatabaseInstance();
    if (db->open(Baloo::Database::OpenDatabase)) {
        Baloo::Transaction tr(db, Baloo::Transaction::ReadOnly);
        m_totalFiles = tr.size();
        m_filesIndexed = tr.size() - tr.phaseOneSize();
        Q_EMIT totalFilesChanged();
    }
}

void Monitor::startBaloo()
{
    const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
    QProcess::startDetached(exe);
}

void Monitor::updateRemainingTime()
{
    m_remainingTime = KFormat().formatSpelloutDuration(m_scheduler->getRemainingTime());
    Q_EMIT remainingTimeChanged();
}

void Monitor::slotIndexerStateChanged(int state)
{
    Baloo::IndexerState newState = static_cast<Baloo::IndexerState>(state);

    if (m_indexerState != newState) {
        m_indexerState = newState;
        Q_EMIT indexerStateChanged();
        fetchTotalFiles();
    }
}

