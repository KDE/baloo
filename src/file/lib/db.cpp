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

#include "db.h"

#include <KDebug>
#include <KStandardDirs>

#include <QSqlQuery>
#include <QSqlError>

std::string fileIndexDbPath()
{
    return KStandardDirs::locateLocal("data", "baloo/file/").toUtf8().constData();
}

QSqlDatabase fileMappingDb() {
    const QString path = KStandardDirs::locateLocal("data", "baloo/file/fileMap.sqlite3");
    return fileMappingDb(path);
}

QSqlDatabase fileMappingDb(const QString& path)
{
    QSqlDatabase sqlDb = QSqlDatabase::database("fileMappingDb");
    if (!sqlDb.isValid()) {
        sqlDb = QSqlDatabase::addDatabase("QSQLITE", "fileMappingDb");
        sqlDb.setDatabaseName(path);
    }

    if (!sqlDb.open()) {
        kDebug() << "Failed to open db" << sqlDb.lastError().text();
        return sqlDb;
    }

    const QStringList tables = sqlDb.tables();
    if (tables.contains("files")) {
        return sqlDb;
    }

    QSqlQuery query(sqlDb);
    bool ret = query.exec("CREATE TABLE files("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "url TEXT NOT NULL UNIQUE)");
    if (!ret) {
        kDebug() << "Could not create tags table" << query.lastError().text();
        return sqlDb;
    }

    ret = query.exec("CREATE INDEX fileUrl_index ON files (url)");
    if (!ret) {
        kDebug() << "Could not create tags index" << query.lastError().text();
        return sqlDb;
    }

    return sqlDb;
}

QSqlDatabase fileMetadataDb()
{
    const QString path = KStandardDirs::locateLocal("data", "baloo/fileMetaData.sqlite3");
    QSqlDatabase sqlDb = QSqlDatabase::database("fileMetadataDb");
    if (!sqlDb.isValid()) {
        sqlDb = QSqlDatabase::addDatabase("QSQLITE", "fileMetadataDb");
        sqlDb.setDatabaseName(path);
    }

    if (!sqlDb.open()) {
        kDebug() << "Failed to open db" << sqlDb.lastError().text();
        return sqlDb;
    }

    const QStringList tables = sqlDb.tables();
    if (tables.contains("files")) {
        return sqlDb;
    }

    QSqlQuery query(sqlDb);
    bool ret = query.exec("CREATE TABLE files("
                          "id INTEGER NOT NULL, "
                          "property TEXT NOT NULL, "
                          "value TEXT NOT NULL, "
                          "UNIQUE(id, property) ON CONFLICT REPLACE)");

    if (!ret) {
        kDebug() << "Could not create tags table" << query.lastError().text();
        return sqlDb;
    }

    ret = query.exec("CREATE INDEX fileprop_index ON files (property)");
    if (!ret) {
        kDebug() << "Could not create tags index" << query.lastError().text();
        return sqlDb;
    }

    return sqlDb;

}

