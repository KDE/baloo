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

#ifndef BALOO_INDEXINGLEVELDB_H
#define BALOO_INDEXINGLEVELDB_H

#include "engine_export.h"
#include <QByteArray>
#include <QVector>
#include <lmdb.h>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentIdDB
{
public:
    explicit DocumentIdDB(MDB_txn* txn);
    ~DocumentIdDB();

    void put(quint64 docId);
    bool contains(quint64 docId);
    void del(quint64 docID);

    QVector<quint64> fetchItems(int size);
    uint size();

    void setTransaction(MDB_txn* txn) {
        m_txn = txn;
    }
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_INDEXINGLEVELDB_H
