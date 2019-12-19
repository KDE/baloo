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

#include <QEventLoop>
#include <QElapsedTimer>
#include <QDBusConnection>

using namespace Baloo;

FileContentIndexer::FileContentIndexer(FileIndexerConfig* config, FileContentIndexerProvider* provider, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_batchSize(config->maxUncomittedFiles())
    , m_provider(provider)
    , m_stop(0)
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
    connect(&process, &ExtractorProcess::startedIndexingFile, this, &FileContentIndexer::slotStartedIndexingFile);
    connect(&process, &ExtractorProcess::finishedIndexingFile, this, &FileContentIndexer::slotFinishedIndexingFile);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    m_stop.store(false);
#else
    m_stop.storeRelaxed(false);
#endif
    auto batchSize = m_batchSize;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    while (m_provider->size() && !m_stop.load()) {
#else
    while (m_provider->size() && !m_stop.loadRelaxed()) {
#endif
        //
        // WARNING: This will go mad, if the Extractor does not commit after N=m_batchSize files
        // cause then we will keep fetching the same N files again and again.
        //
        QElapsedTimer timer;
        timer.start();

        QVector<quint64> idList = m_provider->fetch(batchSize);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        if (idList.isEmpty() || m_stop.load()) {
#else
        if (idList.isEmpty() || m_stop.loadRelaxed()) {
#endif
            break;
        }
        QEventLoop loop;
        connect(&process, &ExtractorProcess::done, &loop, &QEventLoop::quit);

        bool hadErrors = false;
        connect(&process, &ExtractorProcess::failed, &loop, [&hadErrors, &loop]() { hadErrors = true; loop.quit(); });

        process.index(idList);
        loop.exec();

        // QDbus requires us to be in object creation thread (thread affinity)
        // This signal is not even exported, and yet QDbus complains. QDbus bug?
        QMetaObject::invokeMethod(this, "newBatchTime", Qt::QueuedConnection, Q_ARG(uint, timer.elapsed()));
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        if (hadErrors && !m_stop.load()) {
#else
        if (hadErrors && !m_stop.loadRelaxed()) {
#endif
            if (idList.size() == 1) {
                auto failedId = idList.first();
                m_provider->markFailed(failedId);
                batchSize = m_batchSize;
            } else {
                batchSize = idList.size() / 2;
            }
            process.start();
        }
    }
    QMetaObject::invokeMethod(this, "done", Qt::QueuedConnection);
}

void FileContentIndexer::slotStartedIndexingFile(const QString& filePath)
{
    m_currentFile = filePath;
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT startedIndexingFile(filePath);
    }
}

void FileContentIndexer::slotFinishedIndexingFile(const QString& filePath)
{
    Q_UNUSED(filePath);
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT finishedIndexingFile(filePath);
    }
    m_currentFile = QString();
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

void FileContentIndexer::monitorClosed(const QString& service)
{
    m_registeredMonitors.removeAll(service);
    m_monitorWatcher.removeWatchedService(service);
}
