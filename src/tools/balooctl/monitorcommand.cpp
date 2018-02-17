/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "monitorcommand.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>

using namespace Baloo;

MonitorCommand::MonitorCommand(QObject *parent)
    : QObject(parent)
    , m_out(stdout)
    , m_err(stderr)
    , m_dbusInterface(nullptr)

{
    QString waitMessage(i18nc("Application", "Waiting for %1 to start", "Baloo"));
    QString runningMessage(i18nc("Application", "%1 is running", "Baloo"));
    QString diedMessage(i18nc("Application", "%1 died", "Baloo"));
    QString killMessage(i18n("Press ctrl+c to exit monitor"));
    m_dbusInterface = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
        QStringLiteral("/fileindexer"),
        QDBusConnection::sessionBus(),
        this
    );

    m_err << killMessage << endl;
    if (!m_dbusInterface->isValid()) {
        m_err << waitMessage << endl;
    }
    while (!m_dbusInterface->isValid()) {
        QThread::msleep(50);
        m_dbusInterface->disconnect();
        delete m_dbusInterface;
        m_dbusInterface = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
            QStringLiteral("/fileindexer"),
            QDBusConnection::sessionBus(),
            this
        );
    };
    
    m_err << runningMessage << endl;
    m_dbusInterface->registerMonitor();
    connect(m_dbusInterface, &org::kde::baloo::fileindexer::startedIndexingFile,
        this, &MonitorCommand::startedIndexingFile);
    connect(m_dbusInterface, &org::kde::baloo::fileindexer::finishedIndexingFile,
        this, &MonitorCommand::finishedIndexingFile);
    
    auto balooWatcher = new QDBusServiceWatcher(QStringLiteral("org.kde.baloo"), QDBusConnection::sessionBus());
    balooWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(balooWatcher, &QDBusServiceWatcher::serviceUnregistered, [this, diedMessage]() {
            m_out << diedMessage << endl;
            //TODO: Wait again for dbus interface
            QCoreApplication::exit(0);
        });
}

int MonitorCommand::exec(const QCommandLineParser& parser)
{
    Q_UNUSED(parser);
    return QCoreApplication::instance()->exec();
}

void MonitorCommand::startedIndexingFile(const QString& filePath)
{
    m_currentFile = filePath;
    m_out << i18nc("currently indexed file", "Indexing: %1", filePath);
}

void MonitorCommand::finishedIndexingFile(const QString& filePath)
{
    Q_UNUSED(filePath);

    m_currentFile.clear();
    m_out << i18n(": Ok") << endl;
}
