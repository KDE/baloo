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

#include "eventmonitor.h"

#include <QDebug>
#include <KIdleTime>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

// TODO: Make idle timeout configurable?
static int s_idleTimeout = 1000 * 60 * 2; // 2 min

using namespace Baloo;

EventMonitor::EventMonitor(QObject* parent)
    : QObject(parent)
{
    // monitor the powermanagement to not drain the battery
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("/org/freedesktop/PowerManagement"),
                                          QStringLiteral("org.freedesktop.PowerManagement"),
                                          QStringLiteral("PowerSaveStatusChanged"),
                                          this, SLOT(slotPowerManagementStatusChanged(bool)));

    // setup idle time
    KIdleTime* idleTime = KIdleTime::instance();
    connect(idleTime, SIGNAL(timeoutReached(int)), this, SLOT(slotIdleTimeoutReached()));
    connect(idleTime, SIGNAL(resumingFromIdle()), this, SLOT(slotResumeFromIdle()));

    m_isOnBattery = true;
    m_isIdle = false;
    m_enabled = false;

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


EventMonitor::~EventMonitor()
{
}


void EventMonitor::slotPowerManagementStatusChanged(bool conserveResources)
{
    m_isOnBattery = conserveResources;
    if (m_enabled) {
        Q_EMIT powerManagementStatusChanged(conserveResources);
    }
}


void EventMonitor::enable()
{
    /* avoid add multiple idle timeout */
    if (!m_enabled) {
        m_enabled = true;
        KIdleTime::instance()->addIdleTimeout(s_idleTimeout);
    }
}

void EventMonitor::disable()
{
    if (m_enabled) {
        m_enabled = false;
        KIdleTime::instance()->removeAllIdleTimeouts();
    }
}

void EventMonitor::slotIdleTimeoutReached()
{
    if (m_enabled) {
        m_isIdle = true;
        Q_EMIT idleStatusChanged(true);
    }
    KIdleTime::instance()->catchNextResumeEvent();
}

void EventMonitor::slotResumeFromIdle()
{
    m_isIdle = false;
    if (m_enabled) {
        Q_EMIT idleStatusChanged(false);
    }
}
