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

#ifndef BALOO_MTIMEDB_H
#define BALOO_MTIMEDB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QVector>
#include <QMap>

namespace Baloo {

class PostingIterator;

/**
 * The MTime DB maps the file mtime to its id. This allows us to do
 * fast searches of files between a certain time range.
 */
class BALOO_ENGINE_EXPORT MTimeDB
{
public:
    explicit MTimeDB(MDB_dbi dbi, MDB_txn* txn);
    ~MTimeDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(quint32 mtime, quint64 docId);
    QVector<quint64> get(quint32 mtime);

    void del(quint32 mtime, quint64 docId);

    enum Comparator {
        Equal,
        LessEqual,
        GreaterEqual
    };
    PostingIterator* iter(quint32 mtime, Comparator com);
    PostingIterator* iterRange(quint32 beginTime, quint32 endTime);

    QMap<quint32, quint64> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};
}

#endif // BALOO_MTIMEDB_H
