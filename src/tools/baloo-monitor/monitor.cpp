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
    qRegisterMetaType<Baloo::IndexerState>("Baloo::IndexerState");
    qDBusRegisterMetaType<Baloo::IndexerState>();

    QString balooService = QStringLiteral("org.kde.baloo");
    QString extractorService = QStringLiteral("org.kde.baloo.extractor");

    m_balooInterface = new org::kde::balooInterface(balooService,
                                            QStringLiteral("/indexer"),
                                            m_bus, this);

    m_extractorInterface = new org::kde::baloo::extractorInterface(extractorService,
                                                        QStringLiteral("/extractor"),
                                                        m_bus, this);

    connect(m_extractorInterface, &org::kde::baloo::extractorInterface::currentUrlChanged,
            this, &Monitor::newFile);

    connect(m_balooInterface, &org::kde::balooInterface::stateChanged, this, &Monitor::slotIndexerStateChanged);
    qDebug() << "start service watcher";
    QDBusServiceWatcher *balooWatcher = new QDBusServiceWatcher(extractorService,
                                                                m_bus,
                                                                QDBusServiceWatcher::WatchForRegistration,
                                                                this);

    if (m_balooInterface->isValid()) {
        // baloo is already running
        balooStarted(balooService);
        if (m_extractorInterface->isValid()) {
            balooStarted(extractorService);
        }

    } else {
        m_balooRunning = false;
        balooWatcher->addWatchedService(balooService);
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
    if (service == QStringLiteral("org.kde.baloo")) {
        m_balooRunning = true;

        slotIndexerStateChanged(m_balooInterface->state());
        qDebug() << "fetched suspend state";
        fetchTotalFiles();
        Q_EMIT balooStateChanged();
    } else if(service == QStringLiteral("org.kde.baloo.extractor")) {

        /*
         * FIXME: This is pretty useless and slowing up startup by blocking.
         * The interface gives us url only when baloo_extractor is in the event loop
         * hence we only get it at the end of a batch and url is always of the last
         * file in the batch.
         */
        m_url = m_extractorInterface->currentUrl();
        qDebug() << "fetched currentUrl";
        m_extractorInterface->registerMonitor();
        Q_EMIT newFileIndexed();
    }
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
    hms << QStringLiteral(" hour ") << QStringLiteral(" minute ")  << QStringLiteral(" second ");

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

void Monitor::slotIndexerStateChanged(Baloo::IndexerState state)
{
    if (m_indexerState != state) {
        m_indexerState = state;
        Q_EMIT indexerStateChanged();
        fetchTotalFiles();
    }
}

