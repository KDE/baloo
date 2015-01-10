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

#include <QDebug>

using namespace Baloo;

PostingDB::PostingDB(MDB_env* env, MDB_txn* txn)
    : m_env(env)
    , m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "postingdb", MDB_CREATE, &m_dbi);
    Q_ASSERT(rc == 0);
}

PostingDB::~PostingDB()
{
    mdb_dbi_close(m_env, m_dbi);
}

void PostingDB::put(const QByteArray& term, const PostingList& list)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    val.mv_size = list.size() * sizeof(int);
    val.mv_data = static_cast<void*>(const_cast<int*>(list.constData()));

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT(rc == 0);
}

PostingList PostingDB::get(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return PostingList();
    }
    Q_ASSERT(rc == 0);


    PostingList list;
    list.reserve(val.mv_size);

    for (int i = 0; i < (val.mv_size / sizeof(int)); i++) {
        list << static_cast<int*>(val.mv_data)[i];
    }
    return list;
}

DBPostingIterator* PostingDB::iter(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT(rc == 0);

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

uint DBPostingIterator::docId()
{
    uint size = m_size / sizeof(int);
    if (m_pos < 0 || m_pos >= size) {
        return 0;
    }

    int* arr = static_cast<int*>(m_data);
    return arr[m_pos];
}

uint DBPostingIterator::next()
{
    m_pos++;
    uint size = m_size / sizeof(int);
    if (m_pos >= size) {
        return 0;
    }

    int* arr = static_cast<int*>(m_data);
    return arr[m_pos];
}

