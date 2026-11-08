/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_SESSIONIDLEMONITOR_H
#define BALOO_SESSIONIDLEMONITOR_H

#include <QObject>

#include <QDBusServiceWatcher>

namespace Baloo
{

class SessionIdleMonitor : public QObject
{
    Q_OBJECT
public:
    SessionIdleMonitor(QObject *parent = nullptr);
    void updateIdleState();

Q_SIGNALS:
    void idleChanged(bool isIdle);

private Q_SLOTS:
    void idlePropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

private:
    QDBusServiceWatcher m_monitorWatcher;
    bool m_sessionIdle = false;
    bool m_idleMonitorRegistered = false;
};

} // namespace Baloo

#endif // define BALOO_SESSIONIDLEMONITOR_H
