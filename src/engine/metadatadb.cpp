/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "metadatadb.h"
#include "enginedebug.h"

using MetadataDB = Baloo::MetadataDB;
using namespace std::string_literals;

MetadataDB::MetadataDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

MetadataDB::~MetadataDB()
{
}

MDB_dbi MetadataDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "metadatadb", MDB_CREATE, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "MetadataDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi MetadataDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "metadatadb", 0, &dbi);
    if (rc) {
        qCDebug(ENGINE) << "MetadataDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

Baloo::DbVersion MetadataDB::getDbVersion() const
{
    MDB_val key;
    auto dbVersionKey{"DbVersion"s};
    key.mv_size = dbVersionKey.size();
    key.mv_data = dbVersionKey.data();

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "MetadataDB::getVersion" << mdb_strerror(rc);
        }
        return {};
    }

    if (val.mv_size == 16) {
        return DbVersion::make(std::span<const char, 16>(static_cast<const char *>(val.mv_data), 16));
    }

    return {};
}

void MetadataDB::setDbVersion(const Baloo::DbVersion& version)
{
    MDB_val key;
    auto dbVersionKey{"DbVersion"s};
    key.mv_size = dbVersionKey.size();
    key.mv_data = dbVersionKey.data();

    auto serialized = version.serialize();

    MDB_val val;
    val.mv_size = serialized.size();
    val.mv_data = serialized.data();

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCDebug(ENGINE) << "MetadataDB::setVersion" << mdb_strerror(rc);
    }
}
