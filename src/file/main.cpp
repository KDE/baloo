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

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include <KConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

#include "filewatch.h"
#include "fileindexer.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "priority.h"
#include "migrator.h"

#include <QDBusConnection>
#include <QApplication>
#include <QSessionManager>

int main(int argc, char** argv)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KAboutData aboutData(QLatin1String("baloo_file"), i18n("Baloo File"), PROJECT_VERSION,
                         i18n("An application to handle file metadata"),
                         KAboutLicense::LGPL_V2);
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QLatin1String("me@vhanda.in"), QLatin1String("http://vhanda.in"));

    KAboutData::setApplicationData(aboutData);

    QApplication::setDesktopSettingsAware(false);
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    Baloo::FileIndexerConfig indexerConfig;
    if (!indexerConfig.indexingEnabled()) {
        std::cout << "Baloo File Indexing has been disabled" << std::endl;
        return 0;
    }


    if (!QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.baloo"))) {
        qWarning() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    // Crash Handling
    KCrash::setFlags(KCrash::AutoRestart);

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo");

    Baloo::Migrator migrator(path, &indexerConfig);
    if (migrator.migrationRequired()) {
        migrator.migrate();
    }

    Baloo::Database db(path);
    db.open(Baloo::Database::CreateDatabase);

    Baloo::FileWatch filewatcher(&db, &indexerConfig, &app);
    Baloo::FileIndexer fileIndexer(&db, &indexerConfig, &app);

    QObject::connect(&filewatcher, &Baloo::FileWatch::indexFile, &fileIndexer, &Baloo::FileIndexer::indexFile);
    QObject::connect(&filewatcher, &Baloo::FileWatch::indexXAttr, &fileIndexer, &Baloo::FileIndexer::indexXAttr);
    QObject::connect(&filewatcher, &Baloo::FileWatch::installedWatches, &fileIndexer, &Baloo::FileIndexer::update);

    return app.exec();
}
