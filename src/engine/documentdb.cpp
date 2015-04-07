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

#include "documentdb.h"
#include "doctermscodec.h"

#include <QDebug>

using namespace Baloo;

DocumentDB::DocumentDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != 0);
    Q_ASSERT(dbi != 0);
}

DocumentDB::~DocumentDB()
{
}

MDB_dbi DocumentDB::create(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, name, MDB_CREATE | MDB_INTEGERKEY, &dbi);
    Q_ASSERT_X(rc == 0, "DocumentDB::create", mdb_strerror(rc));

    return dbi;
}

MDB_dbi DocumentDB::open(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, name, MDB_CREATE | MDB_INTEGERKEY, &dbi);
    Q_ASSERT_X(rc == 0, "DocumentDB::open", mdb_strerror(rc));

    return dbi;
}

void DocumentDB::put(quint64 docId, const QVector<QByteArray>& list)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    DocTermsCodec codec;
    QByteArray arr = codec.encode(list);

    MDB_val val;
    val.mv_size = arr.size();
    val.mv_data = static_cast<void*>(arr.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "DocumentDB::put", mdb_strerror(rc));
}

QVector<QByteArray> DocumentDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return QVector<QByteArray>();
    }
    Q_ASSERT_X(rc == 0, "DocumentDB::get", mdb_strerror(rc));

    QByteArray arr = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);

    DocTermsCodec codec;
    return codec.decode(arr);
}

void DocumentDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    if (rc == MDB_NOTFOUND) {
        return;
    }
    Q_ASSERT_X(rc == 0, "DocumentDB::del", mdb_strerror(rc));
}

bool DocumentDB::contains(quint64 docId)
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
    Q_ASSERT_X(rc == 0, "DocumentDB::contains", mdb_strerror(rc));

    return true;
}

uint DocumentDB::size()
{
    MDB_stat stat;
    int rc = mdb_stat(m_txn, m_dbi, &stat);
    Q_ASSERT_X(rc == 0, "DocumentDB::size", mdb_strerror(rc));

    return stat.ms_entries;
}
