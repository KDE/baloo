/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "monitorcommand.h"
#include "filecontentindexer.h"
#include "indexerstate.h"

#include <KLazyLocalizedString>

#include <QDBusConnection>
#include <QDBusServiceWatcher>

using namespace Baloo;

namespace
{
using FileStatus = Baloo::IndexResult::FileStatus;
struct {
    FileStatus code;
    KLazyLocalizedString status;
} static constexpr fileResultStrings[] = {
    // clang-format off
    {FileStatus::Successful,                 kli18nc("File index suceeded", "Ok")},
    {FileStatus::IgnoredFilename,            kli18nc("File index skipped",  "File path ignored")},
    {FileStatus::IgnoredMimetype,            kli18nc("File index skipped",  "File type ignored")},
    {FileStatus::IgnoredMimetypeUnsupported, kli18nc("File index skipped",  "File type not supported")},
    {FileStatus::IgnoredTooLarge,            kli18nc("File index skipped",  "File too large")},
    {FileStatus::ErrorFileNotFound,          kli18nc("File index failed",   "File not found")},
    {FileStatus::ErrorExtractionFailed,      kli18nc("File index failed",   "Failed to extract metadata/content")},
    // clang-format on
};

QString statusCodeToString(IndexResult::FileStatus code)
{
    auto match = std::ranges::find_if(fileResultStrings, [code](const auto &entry) {
        return entry.code == code;
    });
    if (match != std::end(fileResultStrings)) {
        return KLocalizedString(match->status).toString();
    }

    return i18nc("Unknown index result code", "<unknown> [%1]", static_cast<quint32>(code));
}
} // namespace <anonymous>

MonitorCommand::MonitorCommand(QObject *parent)
    : QObject(parent)
    , m_out(stdout)
    , m_err(stderr)
    , m_indexerDBusInterface(nullptr)
    , m_schedulerDBusInterface(nullptr)
    , m_dbusServiceWatcher(nullptr)

{
    m_dbusServiceWatcher = new QDBusServiceWatcher(
        QStringLiteral("org.kde.baloo"), QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForOwnerChange, this
    );
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &MonitorCommand::balooIsAvailable);
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &MonitorCommand::balooIsNotAvailable);

    m_indexerDBusInterface = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
        QStringLiteral("/fileindexer"),
        QDBusConnection::sessionBus(),
        this
    );
    connect(m_indexerDBusInterface, &org::kde::baloo::fileindexer::startedIndexingFile,
        this, &MonitorCommand::startedIndexingFile);
    connect(m_indexerDBusInterface, &org::kde::baloo::fileindexer::finishedIndexingFile1, this, &MonitorCommand::finishedIndexingFile);

    m_schedulerDBusInterface = new org::kde::baloo::scheduler(QStringLiteral("org.kde.baloo"),
        QStringLiteral("/scheduler"),
        QDBusConnection::sessionBus(),
        this
    );
    connect(m_schedulerDBusInterface, &org::kde::baloo::scheduler::stateChanged,
        this, &MonitorCommand::stateChanged);

    if (m_indexerDBusInterface->isValid() && m_schedulerDBusInterface->isValid()) {
        m_err << i18n("Press ctrl+c to stop monitoring\n");
        balooIsAvailable();
        stateChanged(m_schedulerDBusInterface->state());

        auto methodCall = QDBusMessage::createMethodCall( //
            m_indexerDBusInterface->service(),
            m_indexerDBusInterface->path(),
            QStringLiteral("org.freedesktop.DBus.Properties"),
            QStringLiteral("Get"));
        methodCall.setArguments({m_indexerDBusInterface->interface(), QStringLiteral("currentFile")});
        auto pendingCall = m_indexerDBusInterface->connection().asyncCall(methodCall, m_indexerDBusInterface->timeout());
        auto *watcher = new QDBusPendingCallWatcher(pendingCall, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QDBusVariant> state = *w;
            w->deleteLater();

            if (!m_currentFile.isEmpty()) {
                // async startedIndexingFile arrived before method return;
                return;
            }
            if (state.isError()) {
                qDebug() << state.error();
                return;
            } else if (const auto currentFile = state.value().variant().toString(); !currentFile.isEmpty()) {
                startedIndexingFile(currentFile);
            }
        });

    } else {
        balooIsNotAvailable();
    }
}

void MonitorCommand::balooIsNotAvailable()
{
    m_indexerDBusInterface->unregisterMonitor();
    m_err << i18n("Waiting for file indexer to start\n");
    m_err << i18n("Press Ctrl+C to stop monitoring\n");
    m_err.flush();
}

void MonitorCommand::balooIsAvailable()
{
    m_indexerDBusInterface->registerMonitor();
    m_err << i18n("File indexer is running\n");
    m_err.flush();
}

int MonitorCommand::exec(const QCommandLineParser& parser)
{
    Q_UNUSED(parser);
    return QCoreApplication::instance()->exec();
}

void MonitorCommand::startedIndexingFile(const QString& filePath)
{
    if (!m_currentFile.isEmpty()) {
        m_out << '\n';
    }
    m_currentFile = filePath;
    m_out << i18nc("currently indexed file", "Indexing: %1", filePath);
    m_out.flush();
}

void MonitorCommand::finishedIndexingFile(const QString &filePath, int, int result)
{
    IndexResult::FileStatus status{result};
    if (m_currentFile.isEmpty()) {
        m_out << i18nc("currently indexed file", "Indexing: %1", filePath);
    }
    m_currentFile.clear();
    m_out << i18nc("index result", ": %1\n", statusCodeToString(status));
    m_out.flush();
}

void MonitorCommand::stateChanged(int state)
{
    m_out << stateString(state) << '\n';
    m_out.flush();
}

#include "moc_monitorcommand.cpp"
