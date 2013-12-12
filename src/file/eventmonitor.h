/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012 Vishesh Handa <me@vhanda.in>

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

#ifndef _BALOO_FILEINDEXER_EVENT_MONITOR_H_
#define _BALOO_FILEINDEXER_EVENT_MONITOR_H_

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>


namespace Baloo
{

class EventMonitor : public QObject
{
    Q_OBJECT

public:
    EventMonitor(QObject* parent = 0);
    ~EventMonitor();

    bool isIdle()         const {
        return m_isIdle;
    }
    bool isDiskSpaceLow() const {
        return m_isDiskSpaceLow;
    }
    bool isOnBattery()    const {
        return m_isOnBattery;
    }

Q_SIGNALS:
    /**
     * Emitted when the power management status changes.
     *
     * \param conserveResources true if you should conserve resources
     */
    void powerManagementStatusChanged(bool conserveResources);

    /**
     * Emitted when the disk space is low. In this case you
     * should stop indexing immediately.
     */
    void diskSpaceStatusChanged(bool isLow);

    /**
     * Emitted when the system becomes idle
     */
    void idleStatusChanged(bool isIdle);

public Q_SLOTS:
    void enable();
    void disable();
    void suspendDiskSpaceMonitor();
    void resumeDiskSpaceMonitor();

private Q_SLOTS:
    void slotIdleTimeoutReached();
    void slotResumeFromIdle();
    void slotPowerManagementStatusChanged(bool conserveResources);
    void slotCheckAvailableSpace();

private:
    bool m_enabled;
    bool m_isIdle;
    bool m_isDiskSpaceLow;
    bool m_isOnBattery;

    // timer used to periodically check for available space
    QTimer m_availSpaceTimer;
};
}

#endif
