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

#include "connection.h"
#include "connection_p.h"

#include <KStandardDirs>
#include <KDebug>

#include <QThreadStorage>
#include <QSqlError>
#include <QSqlQuery>

using namespace Baloo::Tags;

ConnectionPrivate::ConnectionPrivate(const QString& path)
{
    m_connectionName = QString::number(qrand());

    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", m_connectionName));
    m_db->setDatabaseName(path);

    if (!m_db->open()) {
        kDebug() << "Failed to open db" << m_db->lastError().text();
        return;
    }

    const QStringList tables = m_db->tables();
    if (tables.contains("tags") && tables.contains("tagRelations")) {
        return;
    }

    QSqlQuery query(*m_db);
    bool ret = query.exec("CREATE TABLE tags("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL UNIQUE)");
    if (!ret) {
        kDebug() << "Could not create tags table" << query.lastError().text();
        return;
    }

    ret = query.exec("CREATE TABLE tagRelations ("
                     "tid INTEGER NOT NULL, "
                     "rid TEXT NOT NULL, "
                     "PRIMARY KEY (tid, rid) "
                     "FOREIGN KEY (tid) REFERENCES tags (id))");
    if (!ret) {
        kDebug() << "Could not create tagRelation table" << query.lastError().text();
        return;
    }
}

ConnectionPrivate::~ConnectionPrivate()
{
    delete m_db;
    QSqlDatabase::removeDatabase(m_connectionName);
}

Connection::Connection(QObject* parent)
    : QObject(parent)
{
    const QString path = KStandardDirs::locateLocal("data", "baloo/tags.sqlite3");
    d = new ConnectionPrivate(path);
}

Connection::Connection(ConnectionPrivate* priv)
    : QObject()
    , d(priv)
{
}


Connection::~Connection()
{
    delete d;
}

QThreadStorage<Connection*> connections;

Connection* Connection::defaultConnection()
{
    if (connections.hasLocalData())
        return connections.localData();

    Connection* con = new Connection();
    connections.setLocalData(con);
    return con;
}
