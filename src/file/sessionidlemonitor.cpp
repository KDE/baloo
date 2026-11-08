/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "sessionidlemonitor.h"

#include "baloodebug.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

using namespace Baloo;
using namespace Qt::StringLiterals;

static const auto IDLE_MONITOR_SERVICE{u"org.kde.baloo_idle_monitor"_s};

SessionIdleMonitor::SessionIdleMonitor(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    m_monitorWatcher.setConnection(bus);
    m_monitorWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration | QDBusServiceWatcher::WatchForRegistration);

    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &service) {
        if (service == IDLE_MONITOR_SERVICE) {
            qCDebug(BALOO) << "Idle monitor unregistered";
            m_idleMonitorRegistered = false;
            m_sessionIdle = false;
            return;
        }
    });
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString &service) {
        if (service == IDLE_MONITOR_SERVICE) {
            qCDebug(BALOO) << "Idle monitor registered";
            m_idleMonitorRegistered = true;
            updateIdleState();
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
}

void SessionIdleMonitor::idlePropertiesChanged( //
    [[maybe_unused]] const QString &interface,
    [[maybe_unused]] const QVariantMap &changed,
    [[maybe_unused]] const QStringList &invalidated)
{
    auto vIsIdle = changed.value(u"idle"_s);
    if (auto pIsIdle = get_if<bool>(&vIsIdle)) {
        m_sessionIdle = *pIsIdle;
        Q_EMIT idleChanged(m_sessionIdle);
    }
}

void SessionIdleMonitor::updateIdleState()
{
    if (m_idleMonitorRegistered == true) {
        Q_EMIT idleChanged(m_sessionIdle);
        return;
    }

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
                m_sessionIdle = reply.argumentAt<0>();
                Q_EMIT idleChanged(m_sessionIdle);
            }
            response->deleteLater();
        });
}

#include "moc_sessionidlemonitor.cpp"
