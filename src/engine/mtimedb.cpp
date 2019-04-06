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
#include "enginedebug.h"
#include "vectorpostingiterator.h"
#include <algorithm>

using namespace Baloo;

MTimeDB::MTimeDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

MTimeDB::~MTimeDB()
{
}

MDB_dbi MTimeDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "mtimedb", MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "MTimeDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi MTimeDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "mtimedb", MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "MTimeDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void MTimeDB::put(quint32 mtime, quint64 docId)
{
    if (!docId) {
        qCWarning(ENGINE) << "MTimeDB::put - docId == 0";
        return;
    }

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    MDB_val val;
    val.mv_size = sizeof(quint64);
    val.mv_data = static_cast<void*>(&docId);

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "MTimeDB::put" << mdb_strerror(rc);
    }
}

QVector<quint64> MTimeDB::get(quint32 mtime)
{
    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    QVector<quint64> values;

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val val{0, nullptr};
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCWarning(ENGINE) << "MTimeDB::get" << mtime << mdb_strerror(rc);
        }
        mdb_cursor_close(cursor);
        return values;
    }

    values << *static_cast<quint64*>(val.mv_data);

    while (1) {
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT_DUP);
        if (rc) {
            if (rc != MDB_NOTFOUND) {
                qCWarning(ENGINE) << "MTimeDB::get (loop)" << mtime << mdb_strerror(rc);
            }
            break;
        }
        values << *static_cast<quint64*>(val.mv_data);
    }

    mdb_cursor_close(cursor);
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

void MTimeDB::del(quint32 mtime, quint64 docId)
{
    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = static_cast<void*>(&mtime);

    MDB_val val;
    val.mv_size = sizeof(quint64);
    val.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, &val);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCWarning(ENGINE) << "MTimeDB::del" << mtime << docId << mdb_strerror(rc);
    }
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

    MDB_val val{0, nullptr};
    // Set cursor at first element greater or equal key
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCWarning(ENGINE) << "MTimeDB::iter" << mtime << mdb_strerror(rc);
        }
        mdb_cursor_close(cursor);
        return nullptr;
    }

    QVector<quint64> results;

    if (com == GreaterEqual) {
        results << *static_cast<quint64*>(val.mv_data);
        while (1) {
            rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
            if (rc) {
                if (rc != MDB_NOTFOUND) {
                    qCWarning(ENGINE) << "MTimeDB::iter GreaterEqual (loop)" << mtime << mdb_strerror(rc);
                }
                break;
            }

            results << *static_cast<quint64*>(val.mv_data);
        }
    } else {
        quint32 time = *static_cast<quint32*>(key.mv_data);
        if (time == mtime) {
            // set cursor to last element equal key
            rc = mdb_cursor_get(cursor, &key, &val, MDB_LAST_DUP);
        } else {
            // set cursor to element less than key
            rc = mdb_cursor_get(cursor, &key, &val, MDB_PREV);
        }
        if (rc) {
            if (rc != MDB_NOTFOUND) {
                qCWarning(ENGINE) << "MTimeDB::iter LessEqual" << mtime << mdb_strerror(rc);
            } else {
                return nullptr;
            }
        }
        results << *static_cast<quint64*>(val.mv_data);
        while (1) {
            rc = mdb_cursor_get(cursor, &key, &val, MDB_PREV);
            if (rc) {
                if (rc != MDB_NOTFOUND) {
                    qCWarning(ENGINE) << "MTimeDB::iter LessEqual (loop)" << mtime << mdb_strerror(rc);
                }
                break;
            }

            quint64 id = *static_cast<quint64*>(val.mv_data);
            results.push_front(id);
        }
    }

    mdb_cursor_close(cursor);
    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    return new VectorPostingIterator(results);
}

PostingIterator* MTimeDB::iterRange(quint32 beginTime, quint32 endTime)
{
    if (endTime < beginTime) {
        return nullptr;
    }

    MDB_val key;
    key.mv_size = sizeof(quint32);
    key.mv_data = &beginTime;

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val val{0, nullptr};
    int rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCWarning(ENGINE) << "MTimeDB::iterRange" << beginTime << endTime << mdb_strerror(rc);
        }
        mdb_cursor_close(cursor);
        return nullptr;
    }

    QVector<quint64> results;

    while (1) {
        quint32 time = *static_cast<quint32*>(key.mv_data);
        if (time > endTime) {
            break;
        }
        results << *static_cast<quint64*>(val.mv_data);

        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            if (rc != MDB_NOTFOUND) {
                qCWarning(ENGINE) << "MTimeDB::iterRange (loop)" << beginTime << endTime << mdb_strerror(rc);
            }
            break;
        }
    }

    mdb_cursor_close(cursor);

    if (results.isEmpty()) {
        return nullptr;
    }
    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    return new VectorPostingIterator(results);
}

QMap<quint32, quint64> MTimeDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint32, quint64> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            qCDebug(ENGINE) << "MTimeDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint32 time = *(static_cast<quint32*>(key.mv_data));
        const quint64 id = *(static_cast<quint64*>(val.mv_data));
        map.insert(time, id);
    }

    mdb_cursor_close(cursor);
    return map;
}
