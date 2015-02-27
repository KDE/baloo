/*
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

#ifndef BALOO_DOCUMENTDATADB_H
#define BALOO_DOCUMENTDATADB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QByteArray>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentDataDB
{
public:
    explicit DocumentDataDB(const char* name, MDB_txn* txn);
    ~DocumentDataDB();

    void put(uint docId, const QByteArray& data);
    QByteArray get(uint docId);

    void del(uint docId);

    void setTransaction(MDB_txn* txn) {
        m_txn = txn;
    }
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_DOCUMENTDATADB_H
