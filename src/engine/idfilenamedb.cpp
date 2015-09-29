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

#include "idfilenamedb.h"

using namespace Baloo;

IdFilenameDB::IdFilenameDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != 0);
    Q_ASSERT(dbi != 0);
}

IdFilenameDB::~IdFilenameDB()
{
}

MDB_dbi IdFilenameDB::create(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "idfilename", MDB_CREATE | MDB_INTEGERKEY, &dbi);
    Q_ASSERT_X(rc == 0, "IdFilenameDB::create", mdb_strerror(rc));

    return dbi;
}

MDB_dbi IdFilenameDB::open(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "idfilename", MDB_INTEGERKEY, &dbi);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "IdFilenameDB::open", mdb_strerror(rc));

    return dbi;
}

void IdFilenameDB::put(quint64 docId, const FilePath& path)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!path.name.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    QByteArray data(8 + path.name.size(), Qt::Uninitialized);
    memcpy(data.data(), &path.parentId, 8);
    memcpy(data.data() + 8, path.name.data(), path.name.size());

    MDB_val val;
    val.mv_size = data.size();
    val.mv_data = static_cast<void*>(data.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "IdFilenameDB::put", mdb_strerror(rc));
}

IdFilenameDB::FilePath IdFilenameDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    FilePath path;

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return path;
    }
    Q_ASSERT_X(rc == 0, "IdfilenameDB::get", mdb_strerror(rc));

    path.parentId = static_cast<quint64*>(val.mv_data)[0];
    path.name = QByteArray(static_cast<char*>(val.mv_data) + 8, val.mv_size - 8);

    return path;
}

bool IdFilenameDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return false;
    }
    Q_ASSERT_X(rc == 0, "IdfilenameDB::contains", mdb_strerror(rc));
    return true;
}

void IdFilenameDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    Q_ASSERT_X(rc == 0, "IdfilenameDB::del", mdb_strerror(rc));
}

QMap<quint64, IdFilenameDB::FilePath> IdFilenameDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, 0};
    MDB_val val;

    QMap<quint64, FilePath> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "IdFilenameDB::toTestMap", mdb_strerror(rc));

        const quint64 id = *(static_cast<quint64*>(key.mv_data));

        FilePath path;
        path.parentId = static_cast<quint64*>(val.mv_data)[0];
        path.name = QByteArray(static_cast<char*>(val.mv_data) + 8, val.mv_size - 8);

        map.insert(id, path);
    }

    mdb_cursor_close(cursor);
    return map;
}
