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
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

using namespace Baloo;
using namespace Qt::StringLiterals;

static const auto IDLE_MONITOR_SERVICE{u"org.kde.baloo_idle_monitor"_s};

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
    , m_timeEstimator(timeEstimator)
    , m_extractorPath(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/baloo_file_extractor"))
{
    Q_ASSERT(provider);
    qRegisterMetaType<qint32>("Baloo::IndexResult::Changed");
    qRegisterMetaType<qint32>("Baloo::IndexResult::FileStatus");

    QDBusConnection bus = QDBusConnection::sessionBus();
    m_monitorWatcher.setConnection(bus);
    m_monitorWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration | QDBusServiceWatcher::WatchForRegistration);
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &service) {
        if (service == IDLE_MONITOR_SERVICE) {
            qCDebug(BALOO) << "Idle monitor unregistered";
            m_sessionIdle.storeRelaxed(false);
            return;
        }
        qCDebug(BALOO) << "Unregistered monitor" << service;
        m_registeredMonitors.removeAll(service);
        m_monitorWatcher.removeWatchedService(service);
    });
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString &service) {
        if (service == IDLE_MONITOR_SERVICE) {
            qCDebug(BALOO) << "Idle monitor registered";
            fetchIdleState();
        }
    });
    m_monitorWatcher.addWatchedService(IDLE_MONITOR_SERVICE);

    bus.connect(IDLE_MONITOR_SERVICE,
                u"/"_s,
                u"org.freedesktop.DBus.Properties"_s,
                u"PropertiesChanged"_s,
                u"sa{sv}as"_s,
                this,
                SLOT(idlePropertiesChanged(QString, QVariantMap, QStringList)));
    fetchIdleState();

    bus.registerObject(QStringLiteral("/fileindexer"),
                        this, QDBusConnection::ExportScriptableContents);
}

void FileContentIndexer::idlePropertiesChanged( //
    [[maybe_unused]] const QString &interface,
    [[maybe_unused]] const QVariantMap &changed,
    [[maybe_unused]] const QStringList &invalidated)
{
    auto vIsIdle = changed.value(u"idle"_s);
    if (auto pIsIdle = get_if<bool>(&vIsIdle)) {
        sessionIdle(*pIsIdle);
    }
}

void FileContentIndexer::sessionIdle(bool isIdle)
{
    auto oldValue = m_sessionIdle.fetchAndStoreRelaxed(isIdle);
    if (isIdle != oldValue) {
        qCDebug(BALOO) << "Session Idle: " << isIdle;
        Q_EMIT indexSpeedChanged(isIdle);
    }
}

void FileContentIndexer::fetchIdleState()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    auto message = QDBusMessage::createMethodCall( //
        IDLE_MONITOR_SERVICE,
        u"/"_s,
        u"org.freedesktop.DBus.Properties"_s,
        u"Get"_s);
    message.setArguments({u"org.kde.baloo_idle_monitor"_s, u"idle"_s});

    auto pending = bus.asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    QObject::connect( //
        watcher,
        &QDBusPendingCallWatcher::finished,
        this,
        [this](QDBusPendingCallWatcher *response) {
            QDBusPendingReply<bool> reply = *response;
            if (!reply.isError()) {
                bool idle = reply.argumentAt<0>();
                sessionIdle(idle);
            }
            response->deleteLater();
        });
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
    connect(&process, &ExtractorProcess::finishedIndexingFile, this, [this](const QString &path, bool updated, auto status) {
        m_currentFile.clear();
        if (!m_registeredMonitors.empty()) {
            if (status == IndexResult::FileStatus::Successful) {
                auto changed = updated ? IndexResult::Updated : IndexResult::Unchanged;
                Q_EMIT finishedIndexingFile1(path, changed, status);
            } else {
                Q_EMIT finishedIndexingFile1(path, IndexResult::Unchanged, status);
            }
        }
    });
    connect(this, &FileContentIndexer::indexSpeedChanged, &process, [&process](bool indexFast) {
        process.setIndexFast(indexFast);
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
        process.setIndexFast(m_sessionIdle);
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
