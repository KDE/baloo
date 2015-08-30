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

#include "filecontentindexer.h"
#include "filecontentindexerprovider.h"
#include "extractorprocess.h"
#include "timeestimator.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QTimer>
#include <QDBusConnection>

using namespace Baloo;

FileContentIndexer::FileContentIndexer(FileContentIndexerProvider* provider, QObject* parent)
    : QObject(parent)
    , m_provider(provider)
    , m_stop(0)
    , m_delay(0)
    , m_batchTimeBuffer(6, 0)
    , m_bufferIndex(0)
    , m_indexing(0)
{
    Q_ASSERT(provider);

    QDBusConnection bus = QDBusConnection::sessionBus();
    m_monitorWatcher.setConnection(bus);
    m_monitorWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceUnregistered, this,
            &FileContentIndexer::monitorClosed);

    bus.registerObject(QStringLiteral("/fileindexer"),
                        this, QDBusConnection::ExportScriptableContents);
}

void FileContentIndexer::run()
{
    ExtractorProcess process;
    connect(&process, &ExtractorProcess::indexingFile, this, &FileContentIndexer::slotIndexingFile);

    m_stop.store(false);
    m_indexing = true;
    while (m_provider->size() && !m_stop.load()) {
        //
        // WARNING: This will go mad, if the Extractor does not commit after 40 files
        // cause then we will keep fetching the same 40 again and again.
        //
        QElapsedTimer timer;
        timer.start();

        if (m_delay.load()) {
            QTimer delayTimer;
            QEventLoop loop;

            connect(&delayTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
            delayTimer.start(m_delay.load());
            loop.exec();
        }

        QVector<quint64> idList = m_provider->fetch(40);
        if (idList.isEmpty() || m_stop.load()) {
            break;
        }
        QEventLoop loop;
        connect(&process, &ExtractorProcess::done, &loop, &QEventLoop::quit);

        process.index(idList);
        loop.exec();
        // add the current batch time in place of the oldest batch time
        m_batchTimeBuffer[m_bufferIndex % (m_batchTimeBuffer.size() - 1)] = timer.elapsed();
        ++m_bufferIndex;
    }
    m_indexing = false;
    Q_EMIT done();
}

QVector<uint> FileContentIndexer::batchTimings()
{
    if (m_bufferIndex < m_batchTimeBuffer.size() - 1) {
        return QVector<uint>(6, 0);
    }
    // Insert the index of the oldest batch timming as the last entry to let the estimator
    // know which the recentness of each batch.
    m_batchTimeBuffer[m_batchTimeBuffer.size() - 1] = m_bufferIndex % (m_batchTimeBuffer.size() - 1);
    return m_batchTimeBuffer;
}

void FileContentIndexer::slotIndexingFile(QString filePath)
{
    m_currentFile = filePath;
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT indexingFile(filePath);
    }
}

void FileContentIndexer::registerMonitor(const QDBusMessage& message)
{
    if (!m_registeredMonitors.contains(message.service())) {
        m_registeredMonitors << message.service();
        m_monitorWatcher.addWatchedService(message.service());
    }
}

void FileContentIndexer::unregisterMonitor(const QDBusMessage& message)
{
    m_registeredMonitors.removeAll(message.service());
    m_monitorWatcher.removeWatchedService(message.service());
}

void FileContentIndexer::monitorClosed(QString service)
{
    m_registeredMonitors.removeAll(service);
    m_monitorWatcher.removeWatchedService(service);
}

uint FileContentIndexer::getRemainingTime()
{
    if (!m_indexing) {
        return 0;
    }
    TimeEstimator estimator;
    estimator.setFilesLeft(m_provider->size());
    estimator.setBatchTimings(batchTimings());
    return estimator.calculateTimeLeft();
}
