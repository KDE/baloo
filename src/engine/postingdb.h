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
#include <QRegularExpression>

#include <lmdb.h>

namespace Baloo {

typedef QVector<quint64> PostingList;

/**
 * The PostingDB is the main database that maps <term> -> <id1> <id2> <id2> ...
 * This is used to do to lookup ids when searching for a <term>.
 */
class BALOO_ENGINE_EXPORT PostingDB
{
public:
    PostingDB(MDB_dbi, MDB_txn* txn);
    ~PostingDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(const QByteArray& term, const PostingList& list);
    PostingList get(const QByteArray& term);
    void del(const QByteArray& term);

    PostingIterator* iter(const QByteArray& term);
    PostingIterator* prefixIter(const QByteArray& term);
    PostingIterator* regexpIter(const QRegularExpression& regexp, const QByteArray& prefix);

    enum Comparator {
        LessEqual,
        GreaterEqual
    };
    PostingIterator* compIter(const QByteArray& prefix, qlonglong val, Comparator com);
    PostingIterator* compIter(const QByteArray& prefix, const QByteArray& val, Comparator com);

    QVector<QByteArray> fetchTermsStartingWith(const QByteArray& term);

    QMap<QByteArray, PostingList> toTestMap() const;
private:
    template <typename Validator>
    PostingIterator* iter(const QByteArray& prefix, Validator validate);

    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};


}

#endif // BALOO_POSTINGDB_H
