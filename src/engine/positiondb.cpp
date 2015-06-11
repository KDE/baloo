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
#include "orpostingiterator.h"

#include <QDebug>
#include <QRegularExpression>

using namespace Baloo;

PositionDB::PositionDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != 0);
    Q_ASSERT(dbi != 0);
}

PositionDB::~PositionDB()
{
}

MDB_dbi PositionDB::create(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "positiondb", MDB_CREATE, &dbi);
    Q_ASSERT_X(rc == 0, "PositionDB::create", mdb_strerror(rc));

    return dbi;
}

MDB_dbi PositionDB::open(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "positiondb", 0, &dbi);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PositionDB::open", mdb_strerror(rc));

    return dbi;
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

QVector< QByteArray > PositionDB::fetchTermsStartingWith(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    int rc = mdb_cursor_get(cursor, &key, 0, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return QVector<QByteArray>();
    }
    Q_ASSERT_X(rc == 0, "PostingDB::fetchTermsStartingWith", mdb_strerror(rc));

    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(term)) {
        mdb_cursor_close(cursor);
        return QVector<QByteArray>();
    }

    QVector<QByteArray> terms;
    terms << arr;

    while (1) {
        rc = mdb_cursor_get(cursor, &key, 0, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "PostingDB::fetchTermsStartingWith", mdb_strerror(rc));

        const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(term)) {
            break;
        }
        terms << arr;
    }

    mdb_cursor_close(cursor);
    return terms;
}

//
// Query
//

class DBPositionIterator : public PostingIterator {
public:
    DBPositionIterator(void* data, uint size)
        : m_pos(-1)
    {
        PositionCodec codec;
        m_vec = codec.decode(QByteArray::fromRawData(static_cast<char*>(data), size));
    }

    quint64 next() Q_DECL_OVERRIDE {
        m_pos++;
        if (m_pos >= m_vec.size()) {
            return 0;
        }

        return m_vec[m_pos].docId;
    }

    quint64 docId() const Q_DECL_OVERRIDE {
        if (m_pos < 0 || m_pos >= m_vec.size()) {
            return 0;
        }
        return m_vec[m_pos].docId;
    }

    QVector<uint> positions() Q_DECL_OVERRIDE {
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

template <typename Validator>
PostingIterator* PositionDB::iter(const QByteArray& prefix, Validator validate)
{
    Q_ASSERT(!prefix.isEmpty());

    MDB_val key;
    key.mv_size = prefix.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(prefix.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<PostingIterator*> termIterators;

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PostingDB::regexpIter", mdb_strerror(rc));

    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(prefix)) {
        mdb_cursor_close(cursor);
        return 0;
    }
    if (validate(arr)) {
        termIterators << new DBPositionIterator(val.mv_data, val.mv_size);
    }

    while (1) {
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "PostingDB::regexpIter", mdb_strerror(rc));

        const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(prefix)) {
            break;
        }
        if (validate(arr)) {
            termIterators << new DBPositionIterator(val.mv_data, val.mv_size);
        }
    }

    if (termIterators.isEmpty()) {
        mdb_cursor_close(cursor);
        return 0;
    }

    mdb_cursor_close(cursor);
    return new OrPostingIterator(termIterators);
}


PostingIterator* PositionDB::prefixIter(const QByteArray& prefix)
{
    auto validate = [] (const QByteArray& arr) {
        Q_UNUSED(arr);
        return true;
    };
    return iter(prefix, validate);
}

PostingIterator* PositionDB::regexpIter(const QRegularExpression& regexp, const QByteArray& prefix)
{
    int prefixLen = prefix.length();
    auto validate = [&regexp, prefixLen] (const QByteArray& arr) {
        QString term = QString::fromUtf8(arr.mid(prefixLen));
        return regexp.match(term).hasMatch();
    };

    return iter(prefix, validate);
}

PostingIterator* PositionDB::compIter(const QByteArray& prefix, const QByteArray& comVal, PositionDB::Comparator com)
{
    Q_ASSERT(!comVal.isEmpty());
    int prefixLen = prefix.length();
    auto validate = [prefixLen, &comVal, com] (const QByteArray& arr) {
        QByteArray term = QByteArray::fromRawData(arr.constData() + prefixLen, arr.length() - prefixLen);
        return ((com == LessEqual && term <= comVal) || (com == GreaterEqual && term >= comVal));
    };
    return iter(prefix, validate);
}

QMap<QByteArray, QVector<PositionInfo>> PositionDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, 0};
    MDB_val val;

    QMap<QByteArray, QVector<PositionInfo>> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "PostingDB::toTestMap", mdb_strerror(rc));

        const QByteArray ba = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
        const QVector<PositionInfo> vinfo = PositionCodec().decode(QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size));
        map.insert(ba, vinfo);
    }

    mdb_cursor_close(cursor);
    return map;
}
