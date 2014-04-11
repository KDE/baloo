/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <KComponentData>
#include <k4aboutdata.h>
#include <KStandardDirs>
#include <KCmdLineArgs>
#include <KUniqueApplication>
#include <KCrash>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <iostream>

#include "filewatch.h"
#include "fileindexer.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "priority.h"

#include <QDBusConnection>

int main(int argc, char** argv)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    K4AboutData aboutData("baloo_file", "baloo_file", ki18n("Baloo File"), "0.1",
                         ki18n("An application to handle file metadata"),
                         K4AboutData::License_GPL_V2);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KUniqueApplication app(true);
    app.disableSessionManagement();

    KConfig config("baloofilerc");
    KConfigGroup group = config.group("Basic Settings");
    bool indexingEnabled = group.readEntry("Indexing-Enabled", true);

    if (!indexingEnabled) {
        std::cout << "Baloo File Indexing has been disabled" << std::endl;
        return 0;
    }


    if (!QDBusConnection::sessionBus().registerService("org.kde.baloo.file")) {
        kError() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    // Crash Handling
    KCrash::setFlags(KCrash::AutoRestart);

    const QString path = KGlobal::dirs()->localxdgdatadir() + "baloo/file/";

    Database db;
    db.setPath(path);
    db.init();
    db.sqlDatabase().transaction();

    Baloo::FileIndexerConfig indexerConfig;
    Baloo::FileWatch filewatcher(&db, &indexerConfig, &app);

    Baloo::FileIndexer fileIndexer(&db, &indexerConfig, &app);

    QObject::connect(&filewatcher, SIGNAL(indexFile(QString)),
                     &fileIndexer, SLOT(indexFile(QString)));
    QObject::connect(&filewatcher, SIGNAL(installedWatches()),
                     &fileIndexer, SLOT(update()));
    QObject::connect(&filewatcher, SIGNAL(fileRemoved(int)),
                     &fileIndexer, SLOT(removeFileData(int)));

    return app.exec();
}
