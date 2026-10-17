/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

*/

#include <KLocalizedString>
#include <QDBusConnectionInterface>
#include <QTextStream>

#include "purgecommand.h"

#include "global.h"
#include "maininterface.h"

using namespace Baloo;
using namespace Qt::StringLiterals;

static const auto BALOO_SERVICE_NAME{u"org.kde.baloo"_s};

PurgeCommand::PurgeCommand(QObject *parent)
    : QObject(parent)
    , m_mainDBusInterface(std::make_unique<OrgKdeBalooMainInterface>( //
          BALOO_SERVICE_NAME,
          u"/"_s,
          QDBusConnection::sessionBus()))
    , m_dbusServiceWatcher(std::make_unique<QDBusServiceWatcher>( //
          BALOO_SERVICE_NAME,
          QDBusConnection::sessionBus(),
          QDBusServiceWatcher::WatchForOwnerChange,
          this))
    , m_stopTimer(std::make_unique<QTimer>(this))
{
    connect(m_dbusServiceWatcher.get(), //
            &QDBusServiceWatcher::serviceRegistered,
            this,
            &PurgeCommand::balooIsAvailable);
    connect(m_dbusServiceWatcher.get(), //
            &QDBusServiceWatcher::serviceUnregistered,
            this,
            &PurgeCommand::balooIsNotAvailable);

    wasRunning = m_mainDBusInterface->isValid();
}

QString PurgeCommand::command()
{
    return u"purge"_s;
}

QString PurgeCommand::description()
{
    return i18n("Remove the complete DB, and restart the daemon if enabled");
}

void PurgeCommand::balooIsNotAvailable()
{
    QTextStream out(stdout);
    if (wasRunning) {
        m_stopTimer->stop();
        out << " - done\n";
    } else {
        out << "File indexer was not running";
    }

    const QString path = fileIndexDbPath() + QStringLiteral("/index");
    QFile(path).remove();
    out << "Deleted the index database\n";

    if (wasRunning) {
        out << "Restarting the File Indexer";
        out.flush();
        QDBusConnection::sessionBus().interface()->startService(BALOO_SERVICE_NAME);
    } else {
        QCoreApplication::exit(0);
    }
}

void PurgeCommand::balooIsAvailable()
{
    QTextStream out(stdout);
    out << " - restarted\n";
    QCoreApplication::exit(0);
}

int PurgeCommand::exec(const QCommandLineParser &)
{
    QTextStream out(stdout);

    if (wasRunning) {
        out << "Stopping the File Indexer ...";
        out.flush();
        connect(m_stopTimer.get(), &QTimer::timeout, this, []() {
            QTextStream out(stdout);
            out << " - failed to stop!\n";
            QCoreApplication::exit(1);
        });
        m_stopTimer->start(std::chrono::seconds(30));
        m_mainDBusInterface->quit();
    } else {
        QTimer::singleShot(0, [this]() {
            balooIsNotAvailable();
        });
    }

    return QCoreApplication::instance()->exec();
}

#include "moc_purgecommand.cpp"
