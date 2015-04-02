/*
 * This file is part of the KDE Baloo project.
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
#include "positioncodec.h"
#include "positioninfo.h"
#include "postingiterator.h"

#include <QDebug>

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

    PositionCodec codec;
    QByteArray data = codec.encode(list);

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

    PositionCodec codec;
    return codec.decode(data);
}

void PositionDB::del(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    if (rc == MDB_NOTFOUND) {
        return;
    }
    Q_ASSERT_X(rc == 0, "PositionDB::del", mdb_strerror(rc));
}

//
// Query
//

class DBPositionIterator : public PostingIterator {
public:
    DBPositionIterator(char* data, uint size)
        : m_pos(-1)
    {
        PositionCodec codec;
        m_vec = codec.decode(QByteArray::fromRawData(static_cast<char*>(data), size));
    }

    virtual quint64 next() {
        m_pos++;
        if (m_pos >= m_vec.size()) {
            return 0;
        }

        return m_vec[m_pos].docId;
    }

    virtual quint64 docId() {
        if (m_pos < 0 || m_pos >= m_vec.size()) {
            return 0;
        }
        return m_vec[m_pos].docId;
    }

    virtual QVector<uint> positions() {
        if (m_pos < 0 || m_pos >= m_vec.size()) {
            return QVector<uint>();
        }
        return m_vec[m_pos].positions;
    }

private:
    QVector<PositionInfo> m_vec;
    int m_pos;
};

PostingIterator* PositionDB::iter(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PositionDB::iter", mdb_strerror(rc));

    return new DBPositionIterator(static_cast<char*>(val.mv_data), val.mv_size);
}
