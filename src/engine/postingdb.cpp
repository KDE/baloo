/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <me@vhanda.in>
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

#include "postingdb.h"
#include "orpostingiterator.h"
#include "postingcodec.h"

#include <QDebug>

using namespace Baloo;

PostingDB::PostingDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != 0);
    Q_ASSERT(dbi != 0);
}

PostingDB::~PostingDB()
{
}

MDB_dbi PostingDB::create(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "postingdb", MDB_CREATE, &dbi);
    Q_ASSERT_X(rc == 0, "PostingDB::create", mdb_strerror(rc));

    return dbi;
}

MDB_dbi PostingDB::open(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "postingdb", 0, &dbi);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PostingDB::open", mdb_strerror(rc));

    return dbi;
}

void PostingDB::put(const QByteArray& term, const PostingList& list)
{
    Q_ASSERT(!term.isEmpty());
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    PostingCodec codec;
    QByteArray arr = codec.encode(list);

    MDB_val val;
    val.mv_size = arr.size();
    val.mv_data = static_cast<void*>(arr.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "PostingDB::put", mdb_strerror(rc));
}

PostingList PostingDB::get(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return PostingList();
    }
    Q_ASSERT_X(rc == 0, "PostingDB::get", mdb_strerror(rc));

    QByteArray arr = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);

    PostingCodec codec;
    return codec.decode(arr);
}

void PostingDB::del(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    if (rc == MDB_NOTFOUND) {
        return;
    }
    Q_ASSERT_X(rc == 0, "PostingDB::del", mdb_strerror(rc));
}

QVector< QByteArray > PostingDB::fetchTermsStartingWith(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<QByteArray> terms;
    int rc = mdb_cursor_get(cursor, &key, 0, MDB_SET_RANGE);
    while (rc != MDB_NOTFOUND) {
        Q_ASSERT_X(rc == 0, "PostingDB::fetchTermsStartingWith", mdb_strerror(rc));

        const QByteArray arr(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(term)) {
            break;
        }
        terms << arr;
        rc = mdb_cursor_get(cursor, &key, 0, MDB_NEXT);
    }
    Q_ASSERT_X(rc == 0, "PostingDB::fetchTermsStartingWith", mdb_strerror(rc));

    mdb_cursor_close(cursor);
    return terms;
}

class DBPostingIterator : public PostingIterator {
public:
    DBPostingIterator(void* data, uint size);
    quint64 docId() const Q_DECL_OVERRIDE;
    quint64 next() Q_DECL_OVERRIDE;

private:
    const QVector<quint64> m_vec;
    int m_pos;
};

PostingIterator* PostingDB::iter(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PostingDB::iter", mdb_strerror(rc));

    return new DBPostingIterator(val.mv_data, val.mv_size);
}

//
// Posting Iterator
//
DBPostingIterator::DBPostingIterator(void* data, uint size)
    : m_vec(PostingCodec().decode(QByteArray(static_cast<char*>(data), size)))
    , m_pos(-1)
{
}

quint64 DBPostingIterator::docId() const
{
    if (m_pos < 0 || m_pos >= m_vec.size()) {
        return 0;
    }

    return m_vec[m_pos];
}

quint64 DBPostingIterator::next()
{
    if (m_pos >= m_vec.size() - 1) {
        m_pos = m_vec.size();
        return 0;
    }

    m_pos++;
    return m_vec[m_pos];
}

template <typename Validator>
PostingIterator* PostingDB::iter(const QByteArray& prefix, Validator validate)
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
    while (rc != MDB_NOTFOUND) {
        Q_ASSERT_X(rc == 0, "PostingDB::regexpIter", mdb_strerror(rc));

        const QByteArray arr(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(prefix)) {
            break;
        }
        if (validate(arr)) {
            termIterators << new DBPostingIterator(val.mv_data, val.mv_size);
        }
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
    }
    if (rc != MDB_NOTFOUND) {
        Q_ASSERT_X(rc == 0, "PostingDB::regexpIter", mdb_strerror(rc));
    }

    mdb_cursor_close(cursor);
    if (termIterators.isEmpty()) {
        return 0;
    }
    return new OrPostingIterator(termIterators);
}

PostingIterator* PostingDB::prefixIter(const QByteArray& prefix)
{
    auto validate = [] (const QByteArray& arr) {
        Q_UNUSED(arr);
        return true;
    };
    return iter(prefix, validate);
}

PostingIterator* PostingDB::regexpIter(const QRegularExpression& regexp, const QByteArray& prefix)
{
    int prefixLen = prefix.length();
    auto validate = [&regexp, prefixLen] (const QByteArray& arr) {
        QString term = QString::fromUtf8(arr.mid(prefixLen));
        return regexp.match(term).hasMatch();
    };

    return iter(prefix, validate);
}

PostingIterator* PostingDB::compIter(const QByteArray& prefix, const QByteArray& comVal, PostingDB::Comparator com)
{
    Q_ASSERT(!comVal.isEmpty());
    int prefixLen = prefix.length();
    auto validate = [prefixLen, &comVal, com] (const QByteArray& arr) {
        QByteArray term(arr.constData() + prefixLen, arr.length() - prefixLen);
        return ((com == LessEqual && term <= comVal) || (com == GreaterEqual && term >= comVal));
    };
    return iter(prefix, validate);
}

QMap<QByteArray, PostingList> PostingDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, 0};
    MDB_val val;

    QMap<QByteArray, PostingList> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "PostingDB::toTestMap", mdb_strerror(rc));

        const QByteArray ba(static_cast<char*>(key.mv_data), key.mv_size);
        const PostingList plist = PostingCodec().decode(QByteArray(static_cast<char*>(val.mv_data), val.mv_size));
        map.insert(ba, plist);
    }

    mdb_cursor_close(cursor);
    return map;
}
