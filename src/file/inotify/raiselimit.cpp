/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2013 Simeon Bird <bladud@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "raiselimit.h"
#include <QString>
#include <QFile>
#include <QTextStream>

using namespace KAuth;

/* Make the new inotify limit persist across reboots by creating a file in /etc/sysctl.d
 * Using /etc/sysctl.d is much easier than /etc/sysctl.conf, and should work
 * on all systemd distros, debian (including derivatives such as ubuntu) and gentoo.
 * This could potentially be a separate action, but that would require authenticating twice.
 * Also, the user's file system isn't going away - if they wanted a larger limit once, they
 * almost certainly want it again.
 */
bool raiselimitPermanently(int newLimit)
{
    QFile sysctl("/etc/sysctl.d/97-kde-baloo-filewatch-inotify.conf");
    //Just overwrite the existing file.
    if (sysctl.open(QIODevice::WriteOnly)) {
        QTextStream sysc(&sysctl);
        sysc << "fs.inotify.max_user_watches = " << newLimit << "\n";
        sysctl.close();
        return true;
    }
    return false;
}

ActionReply FileWatchHelper::raiselimit(QVariantMap args)
{
    Q_UNUSED(args);

    // Open the procfs file that controls the number of inotify user watches
    QFile inotctl("/proc/sys/fs/inotify/max_user_watches");
    if (!inotctl.open(QIODevice::ReadWrite))
        return ActionReply::HelperErrorReply();
    QTextStream inot(&inotctl);
    // Read the current number
    QString curr = inot.readLine();
    bool ok;
    int now = curr.toInt(&ok);
    if (!ok)
        return ActionReply::HelperErrorReply();
    // Write the new number, which is double the current number
    int next = now*2;
    // Check for overflow
    if (next < now)
        return ActionReply::HelperErrorReply();

    inot << next << "\n";
    inotctl.close();
    raiselimitPermanently(next);
    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.baloo.filewatch", FileWatchHelper)
