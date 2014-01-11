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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

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

bool Database::init()
{
    if (m_initialized)
        return true;

    // Create the Xapian DB
    try {
        Xapian::WritableDatabase(m_path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    }
    catch (const Xapian::DatabaseLockError&) {
    }
    m_xapianDb = new Xapian::Database(m_path.toStdString());

    m_sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE3"));
    m_sqlDb->setDatabaseName(m_path + "/fileMap.sqlite3");

    if (!m_sqlDb->open()) {
        kDebug() << "Failed to open db" << m_sqlDb->lastError().text();
        return false;
    }

    const QStringList tables = m_sqlDb->tables();
    if (tables.contains("files")) {
        return true;
    }

    QSqlQuery query(*m_sqlDb);
    bool ret = query.exec("CREATE TABLE files("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "url TEXT NOT NULL UNIQUE)");
    if (!ret) {
        kDebug() << "Could not create tags table" << query.lastError().text();
        return false;
    }

    ret = query.exec("CREATE INDEX fileUrl_index ON files (url)");
    if (!ret) {
        kDebug() << "Could not create tags index" << query.lastError().text();
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
    if (!m_path.endsWith('/'))
        m_path.append('/');
}

bool Database::isInitialized()
{
    return m_initialized;
}

QSqlDatabase& Database::sqlDatabase()
{
    return *m_sqlDb;
}

Xapian::Database* Database::xapianDatabase()
{
    return m_xapianDb;
}

