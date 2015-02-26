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

#ifndef BALOO_POSTINGDB_H
#define BALOO_POSTINGDB_H

#include "postingiterator.h"

#include <QByteArray>
#include <QVector>
#include <lmdb.h>

namespace Baloo {

typedef QVector<uint> PostingList;

class BALOO_ENGINE_EXPORT PostingDB
{
public:
    explicit PostingDB(MDB_txn* txn);
    ~PostingDB();

    void put(const QByteArray& term, const PostingList& list);
    PostingList get(const QByteArray& term);

    PostingIterator* iter(const QByteArray& term);
    PostingIterator* prefixIter(const QByteArray& term);

    void setTransaction(MDB_txn* txn) {
        m_txn = txn;
    }
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};


}

#endif // BALOO_POSTINGDB_H
