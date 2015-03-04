/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "documenturldb.h"

using namespace Baloo;

DocumentUrlDB::DocumentUrlDB(MDB_txn* txn)
    : m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "documenturldb", MDB_CREATE | MDB_INTEGERKEY, &m_dbi);
    Q_ASSERT_X(rc == 0, "DocumentUrlDB", mdb_strerror(rc));
}

DocumentUrlDB::~DocumentUrlDB()
{
    mdb_dbi_close(mdb_txn_env(m_txn), m_dbi);
}

void DocumentUrlDB::put(quint64 docId, const QByteArray& url)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    val.mv_size = url.size();
    val.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc == MDB_MAP_FULL) {
        Q_ASSERT_X(0, "", "Database is full. You need to increase the map size");
    }
    Q_ASSERT_X(rc == 0, "DocumentUrlDB::put", mdb_strerror(rc));
}

QByteArray DocumentUrlDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return QByteArray();
    }
    Q_ASSERT_X(rc == 0, "DocumentUrlDB::get", mdb_strerror(rc));

    return QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);
}

void DocumentUrlDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    Q_ASSERT_X(rc == 0, "DocumentUrlDB::del", mdb_strerror(rc));
}

uint DocumentUrlDB::size()
{
    MDB_stat stat;
    int rc = mdb_stat(m_txn, m_dbi, &stat);
    Q_ASSERT_X(rc == 0, "DocumentIdDB::size", mdb_strerror(rc));

    return stat.ms_entries;
}
