/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QDBusConnection>
#include <QDBusMessage>
#include <QGuiApplication>
#include <QObject>

#include <KIdleTime>

#include "baloo_idle_monitor_debug.h"

using namespace Qt::StringLiterals;

static const auto IDLE_MONITOR_SERVICE{u"org.kde.baloo_idle_monitor"_s};

namespace Baloo
{
class IdleMonitor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.idle_monitor")
public:
    IdleMonitor()
    {
        m_idleTime->addIdleTimeout(s_idleTimeout);
        connect(m_idleTime, &KIdleTime::timeoutReached, this, [this]() {
            qCDebug(BALOO_IDLE_MONITOR) << "IsIdle";
            m_idleTime->catchNextResumeEvent();
            Q_EMIT idle(true);
        });
        connect(m_idleTime, &KIdleTime::resumingFromIdle, this, [this]() {
            qCDebug(BALOO_IDLE_MONITOR) << "IsBusy";
            Q_EMIT idle(false);
        });
    }
Q_SIGNALS:
    void idle(bool);

private:
    static constexpr int s_idleTimeout = 1000 * 60 * 1; // 1 minute
    KIdleTime *m_idleTime = KIdleTime::instance();
};
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(IDLE_MONITOR_SERVICE)) {
        qCWarning(BALOO_IDLE_MONITOR) << "Failed to register via DBus. Another instance is running";
        return 1;
    }

    Baloo::IdleMonitor monitor{};
    bus.registerObject(QStringLiteral("/"), &monitor, QDBusConnection::ExportAllSignals);

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}

#include "baloo_idle_monitor.moc"
