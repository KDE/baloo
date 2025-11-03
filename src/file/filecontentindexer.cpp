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
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &service) {
        qCDebug(BALOO) << "Unregistered monitor" << service;
        m_registeredMonitors.removeAll(service);
        m_monitorWatcher.removeWatchedService(service);
    });

    bus.registerObject(QStringLiteral("/fileindexer"),
                        this, QDBusConnection::ExportScriptableContents);
}

void FileContentIndexer::run()
{
    ExtractorProcess process{m_extractorPath};

    connect(&process, &ExtractorProcess::startedIndexingFile, this, [this](const QString &path) {
        m_currentFile = path;
        if (!m_registeredMonitors.empty()) {
            Q_EMIT startedIndexingFile(path);
        }
    });
    connect(&process, &ExtractorProcess::finishedIndexingFile, this, [this](const QString &path) {
        m_currentFile.clear();
        if (!m_registeredMonitors.empty()) {
            Q_EMIT finishedIndexingFile(path);
        }
    });

    m_stop.storeRelaxed(false);
    auto batchSize = m_batchSize;
    uint finishedCount = 0;
    uint totalCount = m_provider->size();

    while (true) {
        //
        // WARNING: This will go mad, if the Extractor does not commit after N=m_batchSize files
        // cause then we will keep fetching the same N files again and again.
        //
        QVector<quint64> idList = m_provider->fetch(batchSize);
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

void FileContentIndexer::registerMonitor(const QDBusMessage& message)
{
    auto serviceName = message.service();
    if (!m_registeredMonitors.contains(serviceName)) {
        qCDebug(BALOO) << "Registered monitor" << serviceName;
        m_registeredMonitors << serviceName;
        m_monitorWatcher.addWatchedService(serviceName);
    }
}

void FileContentIndexer::unregisterMonitor(const QDBusMessage& message)
{
    auto serviceName = message.service();
    if (auto count = m_registeredMonitors.removeAll(serviceName); count > 0) {
        m_monitorWatcher.removeWatchedService(serviceName);
        qCDebug(BALOO) << "Unregistered monitor" << serviceName;
    }
}

#include "moc_filecontentindexer.cpp"
