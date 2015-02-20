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

#include "positiondb.h"

#include <QDataStream>

using namespace Baloo;

PositionDB::PositionDB(MDB_txn* txn)
    : m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "positiondb", MDB_CREATE, &m_dbi);
    Q_ASSERT_X(rc == 0, "PositionDB", mdb_strerror(rc));
}

PositionDB::~PositionDB()
{
    mdb_dbi_close(mdb_txn_env(m_txn), m_dbi);
}

void PositionDB::put(const QByteArray& term, const QVector<PositionInfo>& list)
{
    Q_ASSERT(!term.isEmpty());
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (const PositionInfo& pos : list) {
        stream << pos.docId;
        stream << pos.positions;
    }

    MDB_val val;
    val.mv_size = data.size();
    val.mv_data = static_cast<void*>(data.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "PositionDB::put", mdb_strerror(rc));
}

QVector<PositionInfo> PositionDB::get(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return QVector<PositionInfo>();
    }
    Q_ASSERT_X(rc == 0, "PositionDB::get", mdb_strerror(rc));

    QByteArray data = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);
    QDataStream stream(&data, QIODevice::ReadOnly);

    QVector<PositionInfo> vec;
    while (!stream.atEnd()) {
        PositionInfo pos;
        stream >> pos.docId;
        stream >> pos.positions;

        vec << pos;
    }

    return vec;
}
