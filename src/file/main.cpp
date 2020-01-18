/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#include <iostream>

#include "global.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "priority.h"
#include "migrator.h"
#include "mainhub.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QFile>

int main(int argc, char** argv)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KAboutData aboutData(QStringLiteral("baloo"), i18n("Baloo File Indexing Daemon"), PROJECT_VERSION);
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QStringLiteral("vhanda@kde.org"), QStringLiteral("http://vhanda.in"));

    QCoreApplication app(argc, argv);

    KAboutData::setApplicationData(aboutData);

    Baloo::FileIndexerConfig indexerConfig;
    if (!indexerConfig.indexingEnabled()) {
        std::cout << "Baloo File Indexing has been disabled" << std::endl;
        return 0;
    }


    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.baloo"))) {
        qWarning() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    // Crash Handling
    KCrash::setFlags(KCrash::AutoRestart);

    const QString path = Baloo::fileIndexDbPath();

    Baloo::Migrator migrator(path, &indexerConfig);
    if (migrator.migrationRequired()) {
        migrator.migrate();
    }

    if (!QFile::exists(path + "/index")) {
        indexerConfig.setInitialRun(true);
    }

    // HACK: Until we start using lmdb with robust mutex support. We're just going to remove
    //       the lock manually in the baloo_file process.
    QFile::remove(path + "/index-lock");

    Baloo::Database *db = Baloo::globalDatabaseInstance();

    /**
     * try to open, if that fails, try to unlink the index db and retry
     */
    if (!db->open(Baloo::Database::CreateDatabase)) {
        // delete old stuff, set to initial run!
        qWarning() << "Failed to create database, removing corrupted database.";
        QFile::remove(path + "/index");
        QFile::remove(path + "/index-lock");
        indexerConfig.setInitialRun(true);

        // try to create now after cleanup, if still no works => fail
        if (!db->open(Baloo::Database::CreateDatabase)) {
            qWarning() << "Failed to create database after deleting corrupted one.";
            return 1;
        }
    }

    Baloo::MainHub hub(db, &indexerConfig);
    return app.exec();
}
