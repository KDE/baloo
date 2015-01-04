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

#include <QDebug>

using namespace Baloo;

DocumentDB::DocumentDB(MDB_env* env, MDB_txn* txn)
    : m_env(env)
    , m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "documentdb", MDB_CREATE | MDB_INTEGERKEY, &m_dbi);
    Q_ASSERT(rc == 0);
}

DocumentDB::~DocumentDB()
{
    mdb_dbi_close(m_env, m_dbi);
}

void DocumentDB::put(uint docId, const QVector<QByteArray>& list)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(uint);
    key.mv_data = static_cast<void*>(&docId);

    // We need to put this in one huge byte-array
    // FIXME: Ideally, the data should be provided in such a manner. Not in this vector
    QByteArray full;
    for (const QByteArray& ba : list) {
        full.append(ba);
        full.append('\0');
    }

    MDB_val val;
    val.mv_size = full.size();
    val.mv_data = static_cast<void*>(full.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT(rc == 0);
}

QVector<QByteArray> DocumentDB::get(uint docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(uint);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return QVector<QByteArray>();
    }
    Q_ASSERT(rc == 0);

    QVector<QByteArray> list;
    QByteArray full = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);

    int prevWordBoundary = 0;
    for (int i = 0; i < full.size(); i++) {
        if (full[i] == '\0') {
            QByteArray arr = QByteArray::fromRawData(full.constData() + prevWordBoundary,
                                                     i - prevWordBoundary);

            list << arr;
            prevWordBoundary = i + 1;
            i++;
        }
    }

    return list;
}

void DocumentDB::del(uint docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(uint);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    Q_ASSERT(rc == 0);
}
