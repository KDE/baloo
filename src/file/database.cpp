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

#include "database.h"

#include <QStringList>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

Database::Database(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_xapianDb(0)
    , m_sqlDb(0)

{
}

Database::~Database()
{
    QString name = m_sqlDb->connectionName();
    delete m_sqlDb;
    delete m_xapianDb;

    QSqlDatabase::removeDatabase(name);
}

bool Database::init(bool sqlOnly)
{
    if (m_initialized) {
        return true;
    }

    if (!sqlOnly) {
        // Create the Xapian DB
        m_xapianDb = new Baloo::XapianDatabase(m_path);
    }

    m_sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase(QLatin1String("QSQLITE")));
    m_sqlDb->setDatabaseName(m_path + QLatin1String("/fileMap.sqlite3"));
    qDebug() << m_path << QFile::exists(m_path);

    if (!m_sqlDb->open()) {
        qDebug() << "Failed to open db" << m_sqlDb->lastError().text();
        qDebug() << m_sqlDb->lastError();
        return false;
    }

    int version = 1;
    QSqlQuery query(*m_sqlDb);
    bool ret = query.exec(QLatin1String("SELECT version FROM version"));
    if (ret && query.first()) {
        int version = query.value(0).toInt();
        if (version >= CURRENT_SQL_SCHEMA_VERSION) {
            return true;
        }
    }

    if (m_sqlDb->tables().contains(QLatin1String("files"))) {
        return updateSqlSchema(version);
    } else if (version > 1) {
        return updateSqlSchema(version);
    }

    return createSqlSchema();
}

bool Database::updateSqlSchema(int fromVersion)
{
    if (fromVersion == 1) {
        QSqlQuery query(*m_sqlDb);
        bool ret = query.exec("ALTER TABLE files ADD COLUMN indexingLevel int NOT NULL DEFAULT 0");
        if (!ret) {
            qDebug() << "Could not update files table" << query.lastError().text();
            return false;
        }

        // mark all files as beig done; BasicIndexing can reset this as needed
        query.exec("UPDATE files SET indexingLevel = 2");

        ret = query.exec(QLatin1String("CREATE TABLE version (version int NOT NULL)"));
        if (!ret) {
            qDebug() << "Could not create version table" << query.lastError().text();
            return false;
        }
    }

    if (fromVersion < CURRENT_SQL_SCHEMA_VERSION) {
        QSqlQuery query(*m_sqlDb);
        bool ret = query.exec(QLatin1String("DELETE FROM version; INSERT INTO version (version) VALUES (") + QString::number(CURRENT_SQL_SCHEMA_VERSION) + ")");
        if (!ret) {
            qDebug() << "Could not store new version" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool Database::createSqlSchema()
{
    QSqlQuery query(*m_sqlDb);
    bool ret = query.exec(QLatin1String("CREATE TABLE version (version int NOT NULL)"));
    if (!ret) {
        qDebug() << "Could not create version table" << query.lastError().text();
        return false;
    }

    ret = query.exec(QLatin1String("CREATE TABLE files("
                                   "id INTEGER PRIMARY KEY, "
                                   "url TEXT NOT NULL UNIQUE, "
                                   "indexingLevel int NOT NULL DEFAULT 0)"));
    if (!ret) {
        qDebug() << "Could not create tags table" << query.lastError().text();
        return false;
    }

    ret = query.exec(QLatin1String("CREATE INDEX fileUrl_index ON files (url)"));
    if (!ret) {
        qDebug() << "Could not create tags index" << query.lastError().text();
        return false;
    }

    //
    // WAL Journaling mode has much lower io writes than the traditional journal
    // based indexing.
    //
    ret = query.exec(QLatin1String("PRAGMA journal_mode = WAL"));
    if (!ret) {
        qDebug() << "Could not set WAL journaling mode" << query.lastError().text();
        return false;
    }

    m_initialized = true;
    return true;
}

QString Database::path()
{
    return m_path;
}

void Database::setPath(const QString& path)
{
    m_path = path;
    if (!m_path.endsWith(QLatin1Char('/')))
        m_path.append(QLatin1Char('/'));

    QDir().mkpath(m_path);

    QFileInfo dirInfo(path);
    if (!dirInfo.permission(QFile::WriteOwner)) {
        qCritical() << path << "does not have write permissions. Aborting";
        exit(1);
    }
}

bool Database::isInitialized()
{
    return m_initialized;
}

QSqlDatabase& Database::sqlDatabase()
{
    return *m_sqlDb;
}

Baloo::XapianDatabase* Database::xapianDatabase()
{
    return m_xapianDb;
}

