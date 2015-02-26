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

#include "urldocumentdb.h"

#include <QDebug>

using namespace Baloo;

UrlDocumentDB::UrlDocumentDB(MDB_txn* txn)
    : m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "urldocumentdb", MDB_CREATE, &m_dbi);
    Q_ASSERT_X(rc == 0, "UrlDocumentDB", mdb_strerror(rc));
}

UrlDocumentDB::~UrlDocumentDB()
{
    mdb_dbi_close(mdb_txn_env(m_txn), m_dbi);
}

void UrlDocumentDB::put(const QByteArray& url, uint docId)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = url.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    MDB_val val;
    val.mv_size = sizeof(docId);
    val.mv_data = &docId;

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "UrlDocumentDB::put", mdb_strerror(rc));
}

uint UrlDocumentDB::get(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = url.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "UrlDocumentDB::get", mdb_strerror(rc));

    return *static_cast<int*>(val.mv_data);
}

void UrlDocumentDB::del(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = url.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    Q_ASSERT_X(rc == 0, "UrlDocumentDB::del", mdb_strerror(rc));
}

//
// Iter
//

class UrlPostingIterator : public PostingIterator {
public:
    UrlPostingIterator(const QByteArray& url, MDB_cursor* cursor, uint docID)
        : m_url(url), m_cursor(cursor), m_docId(docID), m_first(true)
    {}

    virtual ~UrlPostingIterator() {
        if (m_cursor) {
            mdb_cursor_close(m_cursor);
        }
    }

    virtual uint docId() {
        if (m_first) {
            return 0;
        }
        return m_docId;
    }
    virtual uint next();

private:
    const QByteArray m_url;
    MDB_cursor* m_cursor;
    uint m_docId;
    bool m_first;
};

uint UrlPostingIterator::next()
{
    if (m_first) {
        m_first = false;
        return m_docId;
    }

    MDB_val key;
    MDB_val val;

    int rc = mdb_cursor_get(m_cursor, &key, &val, MDB_NEXT);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(m_cursor);
        m_cursor = 0;
        m_docId = 0;
        return 0;
    }
    Q_ASSERT_X(rc == 0, "UrlPostingIterator::next", mdb_strerror(rc));

    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(m_url)) {
        mdb_cursor_close(m_cursor);
        m_cursor = 0;
        m_docId = 0;
        return 0;
    }

    m_docId = *static_cast<uint*>(val.mv_data);
    return m_docId;
}

PostingIterator* UrlDocumentDB::prefixIter(const QByteArray& url)
{
    MDB_val key;
    key.mv_size = url.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<PostingIterator*> termIterators;

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return 0;
    }
    Q_ASSERT_X(rc == 0, "UrlDocumentDB::prefixIter", mdb_strerror(rc));


    const QByteArray arr = QByteArray::fromRawData(static_cast<char*>(key.mv_data), key.mv_size);
    if (!arr.startsWith(url)) {
        mdb_cursor_close(cursor);
        return 0;
    }

    uint id = *static_cast<uint*>(val.mv_data);
    return new UrlPostingIterator(url, cursor, id);
}
