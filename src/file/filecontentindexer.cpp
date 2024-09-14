/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filecontentindexer.h"
#include "baloodebug.h"
#include "config.h"
#include "extractorprocess.h"
#include "filecontentindexerprovider.h"
#include "timeestimator.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QDBusConnection>

using namespace Baloo;

namespace {
    // TODO KF6 -- remove/combine with started/finished DBus signal
    void sendChangedSignal(const QStringList& updatedFiles)
    {
        auto message = QDBusMessage::createSignal(QStringLiteral("/files"),
                                                  QStringLiteral("org.kde"),
                                                  QStringLiteral("changed"));
        message.setArguments({updatedFiles});
        QDBusConnection::sessionBus().send(message);
    }
}

FileContentIndexer::FileContentIndexer(uint batchSize, //
                                       FileContentIndexerProvider *provider,
                                       TimeEstimator &timeEstimator,
                                       QObject *parent)
    : QObject(parent)
    , m_batchSize(batchSize)
    , m_provider(provider)
    , m_stop(0)
    , m_timeEstimator(timeEstimator)
    , m_extractorPath(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/baloo_file_extractor"))
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
    ExtractorProcess process{m_extractorPath};
    connect(&process, &ExtractorProcess::startedIndexingFile, this, &FileContentIndexer::slotStartedIndexingFile);
    connect(&process, &ExtractorProcess::finishedIndexingFile, this, &FileContentIndexer::slotFinishedIndexingFile);
    m_stop.storeRelaxed(false);
    auto batchSize = m_batchSize;
    uint finishedCount = 0;
    uint totalCount = m_provider->size();

    while (true) {
        //
        // WARNING: This will go mad, if the Extractor does not commit after N=m_batchSize files
        // cause then we will keep fetching the same N files again and again.
        //
        QList<quint64> idList = m_provider->fetch(batchSize);
        if (idList.isEmpty() || m_stop.loadRelaxed()) {
            break;
        }
        QStringList updatedFiles;
        updatedFiles.reserve(idList.size());

        QEventLoop loop;
        connect(&process, &ExtractorProcess::done, &loop, &QEventLoop::quit);

        bool hadErrors = false;
        connect(&process, &ExtractorProcess::failed, &loop, [&hadErrors, &loop]() { hadErrors = true; loop.quit(); });

        auto onFileFinished = [&updatedFiles, &finishedCount, &totalCount, this](const QString &filePath, bool updated) {
            finishedCount++;
            m_timeEstimator.setProgress(totalCount - finishedCount);
            if (updated) {
                updatedFiles.append(filePath);
            }
        };
        connect(&process, &ExtractorProcess::finishedIndexingFile, &loop, onFileFinished, Qt::DirectConnection);

        QElapsedTimer timer;
        timer.start();

        uint batchStartCount = finishedCount;
        process.index(idList);
        loop.exec();
        batchSize = idList.size();

        if (hadErrors && !m_stop.loadRelaxed()) {
            if (batchSize == 1) {
                auto failedId = idList.first();
                auto ok = m_provider->markFailed(failedId);
                if (!ok) {
                    qCCritical(BALOO) << "Not able to commit to DB, DB likely is in a bad state. Exiting";
                    exit(1);
                }
                batchSize = m_batchSize;
            } else {
                batchSize /= 2;
            }
            // reset to old value - nothing committed
            finishedCount = batchStartCount;
            m_timeEstimator.setProgress(totalCount - finishedCount);
            process.start();
        } else {
            // Notify some metadata may have changed after a batch has been finished and committed
            sendChangedSignal(updatedFiles);

            // Update remaining time estimate
            auto elapsed = timer.elapsed();
            QMetaObject::invokeMethod(this,
                [this, elapsed, batchSize] { committedBatch(elapsed, batchSize); },
                Qt::QueuedConnection);
        }
    }
    QMetaObject::invokeMethod(this, &FileContentIndexer::done, Qt::QueuedConnection);
}

void FileContentIndexer::slotStartedIndexingFile(const QString& filePath)
{
    m_currentFile = filePath;
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT startedIndexingFile(filePath);
    }
}

void FileContentIndexer::slotFinishedIndexingFile(const QString &filePath)
{
    m_currentFile = QString();
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT finishedIndexingFile(filePath);
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

void FileContentIndexer::monitorClosed(const QString& service)
{
    m_registeredMonitors.removeAll(service);
    m_monitorWatcher.removeWatchedService(service);
}

#include "moc_filecontentindexer.cpp"
