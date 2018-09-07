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

#ifndef BALOO_DOCUMENTTIMEDB_H
#define BALOO_DOCUMENTTIMEDB_H

#include "engine_export.h"

#include <QByteArray>
#include <QVector>
#include <QMap>
#include <lmdb.h>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentTimeDB
{
public:
    DocumentTimeDB(MDB_dbi dbi, MDB_txn* txn);
    ~DocumentTimeDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    struct TimeInfo {
        quint32 mTime;
        quint32 cTime;

        explicit TimeInfo(quint32 mt = 0, quint32 ct = 0) : mTime(mt), cTime(ct) {}

        bool operator == (const TimeInfo& rhs) const {
            return mTime == rhs.mTime && cTime == rhs.cTime;
        }
    };
    void put(quint64 docId, const TimeInfo& info);
    TimeInfo get(quint64 docId);

    void del(quint64 docId);
    bool contains(quint64 docId);

    QMap<quint64, TimeInfo> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_DOCUMENTTIMEDB_H
