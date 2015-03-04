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

#include "documentiddb.h"

using namespace Baloo;

DocumentIdDB::DocumentIdDB(MDB_txn* txn)
    : m_txn(txn)
{

    int rc = mdb_dbi_open(txn, "indexingleveldb", MDB_CREATE | MDB_INTEGERKEY, &m_dbi);
    Q_ASSERT_X(rc == 0, "IndexingLevelDB", mdb_strerror(rc));
}

DocumentIdDB::~DocumentIdDB()
{
    mdb_dbi_close(mdb_txn_env(m_txn), m_dbi);
}

void DocumentIdDB::put(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    // FIXME: Is the val even required?
    MDB_val val;
    val.mv_size = 0;
    val.mv_data = 0;

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc == MDB_MAP_FULL) {
        Q_ASSERT_X(0, "", "Database is full. You need to increase the map size");
    }
    Q_ASSERT_X(rc == 0, "IndexingLevelDB::put", mdb_strerror(rc));
}

bool DocumentIdDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    // FIXME: We don't need val
    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return false;
    }
    Q_ASSERT_X(rc == 0, "IndexingLevelDB::contains", mdb_strerror(rc));

    return true;
}

void DocumentIdDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    if (rc == MDB_NOTFOUND) {
        return;
    }
    Q_ASSERT_X(rc == 0, "IndexingLevelDB::del", mdb_strerror(rc));
}

QVector<quint64> DocumentIdDB::fetchItems(int size)
{
    Q_ASSERT(size > 0);

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<quint64> vec;

    for (int i = 0; i < size; i++) {
        MDB_val key;
        int rc = mdb_cursor_get(cursor, &key, 0, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "IndexingLevelDB::fetchItems", mdb_strerror(rc));

        quint64 id = *(static_cast<int*>(key.mv_data));
        vec << id;
    }
    mdb_cursor_close(cursor);

    return vec;
}
