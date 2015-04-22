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
#include "database.h"
#include "priority.h"
#include "fileindexerconfig.h"

#include <KConfig>
#include <KConfigGroup>

#include <QStandardPaths>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDir>
#include <QDebug>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    setIdleSchedulingPriority();
    lowerPriority();

    QCoreApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.baloo.file.cleaner"))) {
        qWarning() << "Failed to register via dbus. Another instance is running";
        return 1;
    }

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                         + QStringLiteral("/baloo");

    Baloo::FileIndexerConfig indexerConfig;
    if (!indexerConfig.indexingEnabled()) {
        QFile::remove(path + QStringLiteral("/index"));
        QFile::remove(path + QStringLiteral("/index-lock"));
        return 0;
    }


    Baloo::Database db(path);
    db.open(Baloo::Database::OpenDatabase);

    Baloo::Cleaner cleaner(&db);
    return app.exec();
}
