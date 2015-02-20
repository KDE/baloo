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

using namespace Baloo;

UrlDocumentDB::UrlDocumentDB(MDB_txn* txn)
    : m_txn(txn)
{
    Q_ASSERT(txn != 0);

    int rc = mdb_dbi_open(txn, "urldocumentdb", MDB_CREATE, &m_dbi);
    Q_ASSERT(rc == 0);
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
    Q_ASSERT(rc == 0);
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
    Q_ASSERT(rc == 0);

    return *static_cast<int*>(val.mv_data);
}

void UrlDocumentDB::del(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = url.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    int rc = mdb_del(m_txn, m_dbi, &key, 0);
    Q_ASSERT(rc == 0);
}

