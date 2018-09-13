/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Ashish Bansal <bansal.ashish096@gmail.com>
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

#include "global.h"

#include <QStandardPaths>
using namespace Baloo;

Q_GLOBAL_STATIC_WITH_ARGS(Database, s_db, (fileIndexDbPath()))

QString Baloo::fileIndexDbPath()
{
    QString envBalooPath = QString::fromLocal8Bit(qgetenv("BALOO_DB_PATH"));
    if (!envBalooPath.isEmpty()) {
        return envBalooPath;
    }

    static QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo");
    return path;
}

Database* Baloo::globalDatabaseInstance()
{
    return s_db;
}
