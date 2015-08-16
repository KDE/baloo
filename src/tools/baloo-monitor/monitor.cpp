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

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QStandardPaths>
#include <QDBusServiceWatcher>
#include <QProcess>

using namespace BalooMonitor;
Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_bus(QDBusConnection::sessionBus())
    , m_url(QStringLiteral("Idle"))
    , m_balooInterface(0)
    , m_filesIndexed(0)
    , m_remainingTime(QStringLiteral("Estimating"))
{
    m_balooInterface = new org::kde::balooInterface(QStringLiteral("org.kde.baloo"),
                                            QStringLiteral("/indexer"),
                                            m_bus, this);

    connect(m_balooInterface, &org::kde::balooInterface::indexingFile,
            this, &Monitor::newFile);
    connect(m_balooInterface, &org::kde::balooInterface::stateChanged, this, &Monitor::slotIndexerStateChanged);

    if (m_balooInterface->isValid()) {
        // baloo is already running
        balooStarted(m_balooInterface->service());

    } else {
        m_balooRunning = false;
        QDBusServiceWatcher* balooWatcher = new QDBusServiceWatcher(m_balooInterface->service(),
                                                                m_bus,
                                                                QDBusServiceWatcher::WatchForRegistration,
                                                                this);
        connect(balooWatcher, &QDBusServiceWatcher::serviceRegistered, this, &Monitor::balooStarted);
    }
}

void Monitor::newFile(const QString& url)
{
    if (m_totalFiles == 0) {
        fetchTotalFiles();
    }
    m_url = url;
    if (++m_filesIndexed == m_totalFiles) {
        m_url = QStringLiteral("Done");
    }
    Q_EMIT newFileIndexed();

    if (m_filesIndexed % 200 == 0) {
        updateRemainingTime();
    }
}

QString Monitor::suspendState() const
{
    return m_indexerState == Baloo::Suspended ?  QStringLiteral("Resume") : QStringLiteral("Suspend");
}

void Monitor::toggleSuspendState()
{
    Q_ASSERT(m_balooInterface != 0);

    if (m_indexerState == Baloo::Suspended) {
        m_balooInterface->resume();
    } else {
        m_balooInterface->suspend();
    }
}

void Monitor::balooStarted(const QString& service)
{
    Q_ASSERT(service == QStringLiteral("org.kde.baloo"));

    m_balooRunning = true;
    m_balooInterface->registerMonitor();

    slotIndexerStateChanged(m_balooInterface->state());
    qDebug() << "fetched suspend state";
    fetchTotalFiles();
    if (m_indexerState == Baloo::ContentIndexing) {
        m_url = m_balooInterface->currentFile();
    }
    Q_EMIT balooStateChanged();
}

void Monitor::fetchTotalFiles()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/baloo");
    Baloo::Database db(path);
    db.open(Baloo::Database::OpenDatabase);
    Baloo::Transaction tr(&db, Baloo::Transaction::ReadOnly);
    m_totalFiles = tr.size();
    m_filesIndexed = tr.size() - tr.phaseOneSize();
    Q_EMIT totalFilesChanged();
}

void Monitor::startBaloo()
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file"));
    QProcess::startDetached(exe);
}

void Monitor::updateRemainingTime()
{
    uint seconds = m_balooInterface->getRemainingTime() / 1000;

    QStringList hms;
    hms << QStringLiteral(" hours ") << QStringLiteral(" minutes ")  << QStringLiteral(" seconds ");

    QStringList hms1;
    hms1 << QStringLiteral(" hour ") << QStringLiteral(" minute ")  << QStringLiteral(" second ");

    // time = {h, m, s}
    uint time[] = {0, 0, 0};
    time[0] = seconds / (60 * 60);
    time[1] = (seconds / 60) % 60;
    time[2] = seconds % 60;

    QString strTime;
    for (int i = 0; i < 3; ++i) {
        if (time[i] == 1) {
            strTime += QString::number(time[i]) + hms1.at(i);
        } else if (time[i] != 0) {
            strTime += QString::number(time[i]) + hms.at(i);
        }
    }

    m_remainingTime = strTime;
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

