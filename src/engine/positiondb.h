/*
 * This file is part of the KDE Baloo project.
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

#ifndef BALOO_POSITIONDB_H
#define BALOO_POSITIONDB_H

#include "engine_export.h"

#include <QByteArray>
#include <QMap>
#include <QVector>
#include <lmdb.h>

namespace Baloo {

class PositionInfo;
class VectorPositionInfoIterator;

class BALOO_ENGINE_EXPORT PositionDB
{
public:
    explicit PositionDB(MDB_dbi dbi, MDB_txn* txn);
    ~PositionDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(const QByteArray& term, const QVector<PositionInfo>& list);
    QVector<PositionInfo> get(const QByteArray& term);
    void del(const QByteArray& term);

    VectorPositionInfoIterator* iter(const QByteArray& term);

    QMap<QByteArray, QVector<PositionInfo>> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}


#endif // BALOO_POSITIONDB_H
