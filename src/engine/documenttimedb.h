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

#ifndef BALOO_DOCUMENTIMEDB_H
#define BALOO_DOCUMENTIMEDB_H

#include "engine_export.h"

#include <QByteArray>
#include <QVector>
#include <lmdb.h>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentTimeDB
{
public:
    explicit DocumentTimeDB(MDB_txn* txn);
    ~DocumentTimeDB();

    struct TimeInfo {
        quint64 mTime;
        quint64 cTime;
        quint64 julianDay;

        bool operator == (const TimeInfo& rhs) const {
            return mTime == rhs.mTime && cTime == rhs.cTime && julianDay == rhs.julianDay;
        }
    };
    void put(quint64 docId, const TimeInfo& info);
    TimeInfo get(quint64 docId);

    void del(quint64 docId);

    void setTransaction(MDB_txn* txn) {
        m_txn = txn;
    }
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_DOCUMENTVALUEDB_H
