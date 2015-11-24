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

#include "database.h"
#include "transaction.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"
#include "mtimedb.h"

#include "document.h"
#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "writetransaction.h"
#include "idutils.h"
#include "fsutils.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(0)
{
}

Database::~Database()
{
    mdb_env_close(m_env);
}

bool Database::open(OpenMode mode)
{
    if (isOpen()) {
        return true;
    }

    QDir dir(m_path);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
        dir.refresh();
    }
    QFileInfo indexInfo(dir, QStringLiteral("index"));

    if (mode == OpenDatabase && !indexInfo.exists()) {
        return false;
    }

    if (mode == CreateDatabase) {
        if (!QFileInfo(dir.absolutePath()).permission(QFile::WriteOwner)) {
            qCritical() << m_path << "does not have write permissions. Aborting";
            return false;
        }

        if (!indexInfo.exists()) {
            if (FSUtils::getDirectoryFileSystem(m_path) == QStringLiteral("btrfs")) {
                FSUtils::disableCoW(m_path);
            }
        }
    }

    int rc = mdb_env_create(&m_env);
    if (rc) {
        m_env = 0;
        return false;
    }

    mdb_env_set_maxdbs(m_env, 12);
    mdb_env_set_mapsize(m_env, static_cast<size_t>(1024) * 1024 * 1024 * 5); // 5 gb

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(indexInfo.absoluteFilePath());
    rc = mdb_env_open(m_env, arr.constData(), MDB_NOSUBDIR | MDB_NOMEMINIT, 0664);
    if (rc) {
        m_env = 0;
        return false;
    }

    rc = mdb_reader_check(m_env, 0);
    Q_ASSERT_X(rc == 0, "Database::open reader_check", mdb_strerror(rc));
    if (rc) {
        mdb_env_close(m_env);
        return false;
    }

    //
    // Individual Databases
    //
    MDB_txn* txn;
    if (mode == OpenDatabase) {
        int rc = mdb_txn_begin(m_env, NULL, MDB_RDONLY, &txn);
        Q_ASSERT_X(rc == 0, "Database::transaction ro begin", mdb_strerror(rc));
        if (rc) {
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }

        m_dbis.postingDbi = PostingDB::open(txn);
        m_dbis.positionDBi = PositionDB::open(txn);

        m_dbis.docTermsDbi = DocumentDB::open("docterms", txn);
        m_dbis.docFilenameTermsDbi = DocumentDB::open("docfilenameterms", txn);
        m_dbis.docXattrTermsDbi = DocumentDB::open("docxatrrterms", txn);

        m_dbis.idTreeDbi = IdTreeDB::open(txn);
        m_dbis.idFilenameDbi = IdFilenameDB::open(txn);

        m_dbis.docTimeDbi = DocumentTimeDB::open(txn);
        m_dbis.docDataDbi = DocumentDataDB::open(txn);

        m_dbis.contentIndexingDbi = DocumentIdDB::open("indexingleveldb", txn);
        m_dbis.failedIdDbi = DocumentIdDB::open("failediddb", txn);

        m_dbis.mtimeDbi = MTimeDB::open(txn);

        Q_ASSERT(m_dbis.isValid());
        if (!m_dbis.isValid()) {
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }

        rc = mdb_txn_commit(txn);
        Q_ASSERT_X(rc == 0, "Database::transaction ro commit", mdb_strerror(rc));
        if (rc) {
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }
    } else {
        int rc = mdb_txn_begin(m_env, NULL, 0, &txn);
        Q_ASSERT_X(rc == 0, "Database::transaction begin", mdb_strerror(rc));
        if (rc) {
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }

        m_dbis.postingDbi = PostingDB::create(txn);
        m_dbis.positionDBi = PositionDB::create(txn);

        m_dbis.docTermsDbi = DocumentDB::create("docterms", txn);
        m_dbis.docFilenameTermsDbi = DocumentDB::create("docfilenameterms", txn);
        m_dbis.docXattrTermsDbi = DocumentDB::create("docxatrrterms", txn);

        m_dbis.idTreeDbi = IdTreeDB::create(txn);
        m_dbis.idFilenameDbi = IdFilenameDB::create(txn);

        m_dbis.docTimeDbi = DocumentTimeDB::create(txn);
        m_dbis.docDataDbi = DocumentDataDB::create(txn);

        m_dbis.contentIndexingDbi = DocumentIdDB::create("indexingleveldb", txn);
        m_dbis.failedIdDbi = DocumentIdDB::create("failediddb", txn);

        m_dbis.mtimeDbi = MTimeDB::create(txn);

        Q_ASSERT(m_dbis.isValid());
        if (!m_dbis.isValid()) {
            mdb_txn_abort(txn);
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }

        rc = mdb_txn_commit(txn);
        Q_ASSERT_X(rc == 0, "Database::transaction commit", mdb_strerror(rc));
        if (rc) {
            mdb_env_close(m_env);
            m_env = 0;
            return false;
        }
    }

    return true;
}

QString Database::path() const
{
    return m_path;
}
