/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_METADATADB_H
#define BALOO_METADATADB_H

#include "engine_export.h"

#include "dbversion.h"

#include <QtGlobal>
#include <lmdb.h>

namespace Baloo
{

class BALOO_ENGINE_EXPORT MetadataDB
{
public:
    explicit MetadataDB(MDB_dbi dbi, MDB_txn* txn);
    ~MetadataDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    DbVersion getDbVersion() const;
    void setDbVersion(const DbVersion& version);

private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

} // namespace Baloo

#endif // BALOO_METADATADB_H
