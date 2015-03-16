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

#include <QDebug>

using namespace Baloo;

PostingDB::PostingDB(MDB_txn* txn)
    : m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "postingdb", MDB_CREATE, &m_dbi);
    Q_ASSERT_X(rc == 0, "PostingDB", mdb_strerror(rc));
}

PostingDB::~PostingDB()
{
    mdb_dbi_close(mdb_txn_env(m_txn), m_dbi);
}

void PostingDB::put(const QByteArray& term, const PostingList& list)
{
    Q_ASSERT(!term.isEmpty());
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    val.mv_size = list.size() * sizeof(quint64);
    val.mv_data = static_cast<void*>(const_cast<quint64*>(list.constData()));

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

    PostingList list;
    list.reserve(val.mv_size / sizeof(quint64));

    for (int i = 0; i < (val.mv_size / sizeof(quint64)); i++) {
        list << static_cast<quint64*>(val.mv_data)[i];
    }
    return list;
}

QList<QByteArray> PostingDB::fetchTermsStartingWith(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    int rc = mdb_cursor_get(cursor, &key, 0, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return QList<QByteArray>();
    }
    Q_ASSERT_X(rc == 0, "PostingDB::fetchTermsStartingWith", mdb_strerror(rc));

    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(term)) {
        mdb_cursor_close(cursor);
        return QList<QByteArray>();
    }

    QList<QByteArray> terms;
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

class DBPostingIterator : public PostingIterator {
public:
    DBPostingIterator(void* data, uint size);
    virtual quint64 docId();
    virtual quint64 next();

private:
    void* m_data;
    uint m_size;
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
    : m_data(data)
    , m_size(size)
    , m_pos(-1)
{
}

quint64 DBPostingIterator::docId()
{
    int size = m_size / sizeof(quint64);
    if (m_pos < 0 || m_pos >= size) {
        return 0;
    }

    quint64* arr = static_cast<quint64*>(m_data);
    return arr[m_pos];
}

quint64 DBPostingIterator::next()
{
    m_pos++;
    int size = m_size / sizeof(quint64);
    if (m_pos >= size) {
        return 0;
    }

    quint64* arr = static_cast<quint64*>(m_data);
    return arr[m_pos];
}

PostingIterator* PostingDB::prefixIter(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<PostingIterator*> termIterators;

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return 0;
    }
    Q_ASSERT_X(rc == 0, "PostingDB::prefixIter", mdb_strerror(rc));

    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(term)) {
        mdb_cursor_close(cursor);
        return 0;
    }
    termIterators << new DBPostingIterator(val.mv_data, val.mv_size);

    while (1) {
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "PostingDB::prefixIter", mdb_strerror(rc));

        const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(term)) {
            break;
        }
        termIterators << new DBPostingIterator(val.mv_data, val.mv_size);
    }

    if (termIterators.isEmpty()) {
        mdb_cursor_close(cursor);
        return 0;
    }

    mdb_cursor_close(cursor);
    return new OrPostingIterator(termIterators);
}

PostingIterator* PostingDB::regexpIter(const QRegularExpression& regexp, const QByteArray& prefix)
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
    const QString term = QString::fromUtf8(arr.mid(prefix.length()));
    if (regexp.match(term).hasMatch()) {
        termIterators << new DBPostingIterator(val.mv_data, val.mv_size);
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
        const QString term = QString::fromUtf8(arr.mid(prefix.length()));
        if (regexp.match(term).hasMatch()) {
            termIterators << new DBPostingIterator(val.mv_data, val.mv_size);
        }
    }

    if (termIterators.isEmpty()) {
        mdb_cursor_close(cursor);
        return 0;
    }

    mdb_cursor_close(cursor);
    return new OrPostingIterator(termIterators);
}
