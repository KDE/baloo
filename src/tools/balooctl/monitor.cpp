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

#include <QDBusConnection>

using namespace Baloo;
Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , m_out(stdout)
{
    m_interface = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/fileindexer"),
                                                QDBusConnection::sessionBus(),
                                                this);

    if (!m_interface->isValid()) {
        m_out << "Baloo is not running" << endl;
        QCoreApplication::exit();
    }
    m_interface->registerMonitor();
    connect(m_interface, &org::kde::baloo::fileindexer::indexingFile, this, &Monitor::newFile);
    m_out << "Press ctrl+c to exit monitor" << endl;
}

void Monitor::newFile(const QString& url)
{
    m_out << "Indexing: " << url << endl;
}
