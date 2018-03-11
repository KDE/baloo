/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

   Parts of this file are based on code from Strigi
   Copyright (C) 2006-2007 Jos van den Oever <jos@vandenoever.info>

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

#include "priority.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <QDebug>

#include <sys/time.h>
#include <sys/resource.h>

#include <unistd.h>
#ifndef _WIN32
#include <sys/syscall.h>
#include <errno.h>

#include <sched.h>
#endif

#ifdef SYS_ioprio_set
namespace {
#ifndef IOPRIO_CLASS_IDLE
    enum {
        IOPRIO_CLASS_NONE,
        IOPRIO_CLASS_RT,
        IOPRIO_CLASS_BE,
        IOPRIO_CLASS_IDLE
    };
#endif

#ifndef IOPRIO_WHO_PROCESS
    enum {
        IOPRIO_WHO_PROCESS = 1,
        IOPRIO_WHO_PGRP,
        IOPRIO_WHO_USER
    };
#endif

#ifndef IOPRIO_CLASS_SHIFT
    const int IOPRIO_CLASS_SHIFT = 13;
#endif
}
#endif


bool lowerIOPriority()
{
#ifdef SYS_ioprio_set
    if ( syscall( SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, IOPRIO_CLASS_IDLE<<IOPRIO_CLASS_SHIFT ) < 0 ) {
        qDebug( "cannot set io scheduling to idle (%s). Trying best effort.\n",  strerror( errno ));
        if ( syscall( SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, 7|IOPRIO_CLASS_BE<<IOPRIO_CLASS_SHIFT ) < 0 ) {
            qDebug( "cannot set io scheduling to best effort.\n");
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}


bool lowerPriority()
{
#ifndef Q_OS_WIN
    return !setpriority( PRIO_PROCESS, 0, 19 );
#else
    return false;
#endif
}


bool lowerSchedulingPriority()
{
#ifdef SCHED_BATCH
    struct sched_param param;
    memset( &param, 0, sizeof(param) );
    param.sched_priority = 0;
    return !sched_setscheduler( 0, SCHED_BATCH, &param );
#else
    return false;
#endif
}

bool setIdleSchedulingPriority()
{
#ifdef SCHED_IDLE
    struct sched_param param;
    memset( &param, 0, sizeof(param) );
    param.sched_priority = 0;
    return !sched_setscheduler( 0, SCHED_IDLE, &param );
#else
    return false;
#endif
}
