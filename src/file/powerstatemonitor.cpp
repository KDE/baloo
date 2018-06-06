/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-15 Vishesh Handa <vhanda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "powerstatemonitor.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

using namespace Baloo;

PowerStateMonitor::PowerStateMonitor(QObject* parent)
    : QObject(parent)
    , m_isOnBattery(false)
{
    // monitor the powermanagement to not drain the battery
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("/org/freedesktop/PowerManagement"),
                                          QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("PowerSaveStatusChanged"),
                                          this, SLOT(slotPowerManagementStatusChanged(bool)));


    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement"),
                                                      QStringLiteral("/org/freedesktop/PowerManagement"),
                                                      QStringLiteral("org.freedesktop.PowerManagement"),
                                                      QStringLiteral("GetPowerSaveStatus"));

    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher* watch) {
        QDBusPendingReply<bool> reply = *watch;
        if (!reply.isError()) {
            bool onBattery = reply.argumentAt<0>();
            slotPowerManagementStatusChanged(onBattery);
        }
        watch->deleteLater();
    });
}

void PowerStateMonitor::slotPowerManagementStatusChanged(bool conserveResources)
{
    if (m_isOnBattery != conserveResources) {
        m_isOnBattery = conserveResources;
        Q_EMIT powerManagementStatusChanged(conserveResources);
    }
}
