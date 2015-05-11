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

#include "mtimedb.h"
#include "vectorpostingiterator.h"

using namespace Baloo;

MTimeDB::MTimeDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != 0);
    Q_ASSERT(dbi != 0);
}

MTimeDB::~MTimeDB()
{
}

MDB_dbi MTimeDB::create(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "mtimedb", MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP, &dbi);
    Q_ASSERT_X(rc == 0, "MTimeDB::create", mdb_strerror(rc));

    return dbi;
}

MDB_dbi MTimeDB::open(MDB_txn* txn)
{
    MDB_dbi dbi;
    int rc = mdb_dbi_open(txn, "mtimedb", MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP, &dbi);
    if (rc == MDB_NOTFOUND) {
        return 0;
    }
    Q_ASSERT_X(rc == 0, "MTimeDB::open", mdb_strerror(rc));

    return dbi;
}

void MTimeDB::put(quint32 mtime, quint64 docId)
{
    Q_ASSERT(mtime > 0);
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    MDB_val val;
    val.mv_size = sizeof(quint64);
    val.mv_data = static_cast<void*>(&docId);

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    Q_ASSERT_X(rc == 0, "MTimeDB::put", mdb_strerror(rc));
}

QVector<quint64> MTimeDB::get(quint64 mtime)
{
    Q_ASSERT(mtime > 0);

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    QVector<quint64> values;

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return values;
    }
    Q_ASSERT_X(rc == 0, "MTimeDB::get", mdb_strerror(rc));

    values << *static_cast<quint64*>(val.mv_data);

    while (1) {
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT_DUP);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "MTimeDB::get while", mdb_strerror(rc));

        values << *static_cast<quint64*>(val.mv_data);
    }

    mdb_cursor_close(cursor);
    return values;
}

void MTimeDB::del(quint32 mtime, quint64 docId)
{
    Q_ASSERT(mtime > 0);
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    MDB_val val;
    val.mv_size = sizeof(quint64);
    val.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, &val);
    if (rc == MDB_NOTFOUND) {
        return;
    }
    Q_ASSERT_X(rc == 0, "DocumentDB::del", mdb_strerror(rc));
}

//
// Posting Iterator
//

PostingIterator* MTimeDB::iter(quint32 mtime, MTimeDB::Comparator com)
{
    if (com == Equal) {
        return new VectorPostingIterator(get(mtime));
    }

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = &mtime;

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return 0;
    }
    Q_ASSERT_X(rc == 0, "MTimeDB::iter", mdb_strerror(rc));

    QVector<quint64> results;
    results << *static_cast<quint64*>(val.mv_data);

    if (com == GreaterEqual) {
        while (1) {
            rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
            if (rc == MDB_NOTFOUND) {
                break;
            }
            Q_ASSERT_X(rc == 0, "MTimeDB::iter >=", mdb_strerror(rc));

            results << *static_cast<quint64*>(val.mv_data);
        }
    }
    else {
        while (1) {
            rc = mdb_cursor_get(cursor, &key, &val, MDB_PREV);
            if (rc == MDB_NOTFOUND) {
                break;
            }
            Q_ASSERT_X(rc == 0, "MTimeDB::iter >=", mdb_strerror(rc));

            quint64 id = *static_cast<quint64*>(val.mv_data);
            results.push_front(id);
        }
    }

    mdb_cursor_close(cursor);
    return new VectorPostingIterator(results);
}

PostingIterator* MTimeDB::iterRange(quint32 beginTime, quint32 endTime)
{
    Q_ASSERT(beginTime);
    Q_ASSERT(endTime);

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = &beginTime;

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val val;
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc == MDB_NOTFOUND) {
        mdb_cursor_close(cursor);
        return 0;
    }
    Q_ASSERT_X(rc == 0, "MTimeDB::iterRange", mdb_strerror(rc));

    QVector<quint64> results;
    results << *static_cast<quint64*>(val.mv_data);

    while (1) {
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        Q_ASSERT_X(rc == 0, "MTimeDB::iter >=", mdb_strerror(rc));

        quint32 time = *static_cast<quint32*>(key.mv_data);
        if (time > endTime) {
            break;
        }
        results << *static_cast<quint64*>(val.mv_data);
    }

    mdb_cursor_close(cursor);
    return new VectorPostingIterator(results);
}
