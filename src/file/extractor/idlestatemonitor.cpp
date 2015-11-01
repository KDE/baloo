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

#include "idlestatemonitor.h"

#include <KIdleTime>

// TODO: Make idle timeout configurable?
static int s_idleTimeout = 1000 * 60 * 2; // 2 min

using namespace Baloo;

IdleStateMonitor::IdleStateMonitor(QObject* parent)
    : QObject(parent)
    , m_isIdle(false)
{
    // setup idle time
    KIdleTime* idleTime = KIdleTime::instance();
    idleTime->addIdleTimeout(s_idleTimeout);
    connect(idleTime, SIGNAL(timeoutReached(int)), this, SLOT(slotIdleTimeoutReached()));
    connect(idleTime, &KIdleTime::resumingFromIdle, this, &IdleStateMonitor::slotResumeFromIdle);
}

IdleStateMonitor::~IdleStateMonitor()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleStateMonitor::slotIdleTimeoutReached()
{
    m_isIdle = true;
}

void IdleStateMonitor::slotResumeFromIdle()
{
    m_isIdle = false;
}
