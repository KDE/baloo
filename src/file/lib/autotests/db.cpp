/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "db.h"

#include <QString>
#include <QStringList>

#include <KTempDir>
#include <KDebug>

#include <QSqlQuery>
#include <QSqlError>

static KTempDir dir;

std::string fileIndexDbPath()
{
    return dir.name().toUtf8().constData();
}

// FIXME: Avoid duplicating this code!
QSqlDatabase fileMappingDb()
{
    QSqlDatabase sqlDb = QSqlDatabase::database("fileMappingDb");
    if (!sqlDb.isValid()) {
        sqlDb = QSqlDatabase::addDatabase("QSQLITE", "fileMappingDb");
        sqlDb.setDatabaseName(dir.name() + "fileMap.sqlite3");
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
    QSqlDatabase sqlDb = QSqlDatabase::database("fileMetadataDb");
    if (!sqlDb.isValid()) {
        sqlDb = QSqlDatabase::addDatabase("QSQLITE", "fileMetadataDb");
        sqlDb.setDatabaseName(dir.name() + "fileMetadData.sqlite3");
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
