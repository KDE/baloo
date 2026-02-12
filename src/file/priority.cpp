/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>

    Parts of this file are based on code from Strigi
    SPDX-FileCopyrightText: 2006-2007 Jos van den Oever <jos@vandenoever.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "priority.h"
#include "baloodebug.h"

#include <cerrno>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef SYS_ioprio_set
#include <linux/ioprio.h>
#endif

bool lowerIOPriority()
{
#ifdef SYS_ioprio_set
    if (syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, ioprio_value(IOPRIO_CLASS_IDLE, 0, IOPRIO_HINT_NONE)) >= 0) {
        return true;
    }
    int idle_error = errno;

    if (syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, ioprio_value(IOPRIO_CLASS_BE, 7, IOPRIO_HINT_NONE)) >= 0) {
        qCDebug(BALOO, "Cannot set io scheduling to Idle (%s). Using Best Effort.\n", strerror(idle_error));
        return true;
    }
    qCDebug(BALOO, "Cannot set io scheduling to Best Effort or Idle.\n");
#endif

    return false;
}

bool lowerPriority()
{
    return !setpriority(PRIO_PROCESS, 0, 19);
}

bool lowerSchedulingPriority()
{
#ifdef SCHED_BATCH
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = 0;
    return !sched_setscheduler(0, SCHED_BATCH, &param);
#else
    return false;
#endif
}

bool setIdleSchedulingPriority()
{
#ifdef SCHED_IDLE
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = 0;
    return !sched_setscheduler(0, SCHED_IDLE, &param);
#else
    return false;
#endif
}
