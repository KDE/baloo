/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2014 Vishesh Handa <me@vhanda.in>

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

#include "cleaner.h"
#include "../database.h"
#include "../priority.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KComponentData>
#include <QApplication>
#include <QDBusConnection>

#include <KDebug>
#include <KStandardDirs>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KAboutData aboutData("baloo_file_cleaner", 0, ki18n("Baloo File Cleaner"),
                         "0.1",
                         ki18n("Cleans up stale file index information"),
                         KAboutData::License_LGPL_V2,
                         ki18n("(C) 2014, Vishesh Handa"));
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    QApplication app(argc, argv);
    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    if (!QDBusConnection::sessionBus().registerService("org.kde.baloo.file.cleaner")) {
        kError() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    Database db;
    db.setPath(KStandardDirs::locateLocal("data", "baloo/file/"));
    db.init();

    Baloo::Cleaner cleaner(&db);
    return app.exec();
}
