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
    Q_PROPERTY(bool idle MEMBER m_isIdle);

public:
    IdleMonitor()
    {
        m_idleTime->addIdleTimeout(s_idleTimeout);
        connect(m_idleTime, &KIdleTime::timeoutReached, this, [this]() {
            qCDebug(BALOO_IDLE_MONITOR) << "IsIdle";
            m_idleTime->catchNextResumeEvent();
            m_isIdle = true;
            Q_EMIT idle(true);
        });
        connect(m_idleTime, &KIdleTime::resumingFromIdle, this, [this]() {
            qCDebug(BALOO_IDLE_MONITOR) << "IsBusy";
            m_isIdle = false;
            Q_EMIT idle(false);
        });
    }

Q_SIGNALS:
    void idle(bool);

private:
    static constexpr int s_idleTimeout = 1000 * 60 * 1; // 1 minute
    KIdleTime *m_idleTime = KIdleTime::instance();
    bool m_isIdle = false;
};

void dbusSignalIdleChanged(const QDBusConnection &bus, bool isIdle)
{
    auto message = QDBusMessage::createSignal( //
        u"/"_s,
        u"org.freedesktop.DBus.Properties"_s,
        u"PropertiesChanged"_s);

    auto arguments = QList<QVariant>{
        u"/"_s,
        QVariantMap({
            {u"idle"_s, isIdle},
        }),
        QStringList{}, // invalidated
    };

    message.setArguments(arguments);
    bus.send(message);
}
} // namespace

int main(int argc, char **argv)
{
    // Avoid desktop portal registration implied by QGuiApplication
    qputenv("QT_NO_XDG_DESKTOP_PORTAL", "1");

    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(IDLE_MONITOR_SERVICE)) {
        qCWarning(BALOO_IDLE_MONITOR) << "Failed to register via DBus. Another instance is running";
        return 1;
    }

    Baloo::IdleMonitor monitor{};
    bus.registerObject(QStringLiteral("/"), &monitor, QDBusConnection::ExportScriptableProperties);
    QObject::connect(&monitor, &Baloo::IdleMonitor::idle, [bus](bool idle) {
        Baloo::dbusSignalIdleChanged(bus, idle);
    });

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}

#include "baloo_idle_monitor.moc"
