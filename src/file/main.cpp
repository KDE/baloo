/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCrash>

#include <iostream>

#include "baloodebug.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "global.h"
#include "mainhub.h"
#include "migrator.h"
#include "priority.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QFile>

#include <KAboutData>

int main(int argc, char** argv)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    QCoreApplication app(argc, argv);

    Baloo::FileIndexerConfig indexerConfig;
    if (!indexerConfig.indexingEnabled()) {
        std::cout << "Baloo File Indexing has been disabled" << std::endl;
        return 0;
    }

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.baloo"))) {
        qCWarning(BALOO) << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    KAboutData aboutData(QStringLiteral("baloo_file"), QString(), QLatin1String(PROJECT_VERSION));
    KAboutData::setApplicationData(aboutData);

    // Crash Handling
    KCrash::initialize();
    KCrash::setFlags(KCrash::AutoRestart);

    const QString path = Baloo::fileIndexDbPath();

    Baloo::Migrator migrator(path, &indexerConfig);
    if (migrator.migrationRequired()) {
        migrator.migrate();
    }

    bool firstRun = !QFile::exists(path + QStringLiteral("/index"));

    Baloo::Database *db = Baloo::globalDatabaseInstance();

    /**
     * try to open, if that fails, try to unlink the index db and retry
     */
    using OpenResult = Baloo::Database::OpenResult;
    if (auto rc = db->open(Baloo::Database::CreateDatabase); rc != OpenResult::Success) {
        if (rc == OpenResult::InvalidPath) {
            return 1;
        }
        // delete old stuff, set to initial run!
        qCWarning(BALOO) << "Failed to create database, removing corrupted database.";
        QFile::remove(path + QStringLiteral("/index"));
        QFile::remove(path + QStringLiteral("/index-lock"));
        firstRun = true;

        // try to create now after cleanup, if still no works => fail
        if (db->open(Baloo::Database::CreateDatabase) != OpenResult::Success) {
            qCWarning(BALOO) << "Failed to create database after deleting corrupted one.";
            return 1;
        }
    }

    Baloo::MainHub hub(db, &indexerConfig, firstRun);
    return app.exec();
}
