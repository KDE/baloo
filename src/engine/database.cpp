/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "writetransaction.h"
#include "idutils.h"
#include "fsutils.h"

#include "enginedebug.h"

// MSVC does not understand the inline assembly in valgrind.h
// Defining NVALGRIND stubs out all definitions, so we can use
// the macros without ifdef'ing these in place
#if defined _MSC_VER && !defined NVALGRIND
#define NVALGRIND 1
#endif
#include "valgrind.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMutexLocker>

#include <cstdlib>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(nullptr)
{
}

Database::~Database()
{
    // try only to close if we did open the DB successfully
    if (m_env) {
        mdb_env_close(m_env);
        m_env = nullptr;
    }
}

// Damage that comes back after the index has been built anew is a sign of something that
// keeps breaking it, and building it yet again would only feed a loop of reindexing and
// crashing. A record of the damage seen so far tells the two apart. It only holds for as
// long as this window, so an incident a season later counts as a fresh one.
static constexpr int corruptionWindowInDays = 30;

QString Database::corruptionRecordPath(const QString &path)
{
    return path + QStringLiteral("/index-corruption");
}

Database::CorruptionRecord Database::readCorruptionRecord(const QString &path)
{
    CorruptionRecord record;
    QFile file(corruptionRecordPath(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return record;
    }

    const QList<QByteArray> lines = file.readAll().split('\n');
    for (const QByteArray &line : lines) {
        const int separator = line.indexOf('=');
        if (separator < 0) {
            continue;
        }
        const QByteArray key = line.left(separator);
        const QString value = QString::fromUtf8(line.mid(separator + 1));
        if (key == "count") {
            record.count = value.toInt();
        } else if (key == "lastRebuild") {
            record.lastRebuild = QDateTime::fromString(value, Qt::ISODate);
        }
    }
    return record;
}

void Database::writeCorruptionRecord(const QString &path, const CorruptionRecord &record)
{
    QFile file(corruptionRecordPath(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(ENGINE) << "Could not write" << corruptionRecordPath(path) << file.errorString();
        return;
    }
    file.write("count=" + QByteArray::number(record.count) + '\n');
    if (record.lastRebuild.isValid()) {
        file.write("lastRebuild=" + record.lastRebuild.toString(Qt::ISODate).toUtf8() + '\n');
    }
}

void Database::noteCorruption(const QString &path, const QString &reason)
{
    CorruptionRecord record = readCorruptionRecord(path);
    const bool withinWindow = record.lastRebuild.isValid() && record.lastRebuild.daysTo(QDateTime::currentDateTime()) < corruptionWindowInDays;
    record.count = withinWindow ? record.count + 1 : 1;
    writeCorruptionRecord(path, record);

    qCCritical(ENGINE).noquote() << QStringLiteral(
                                        "The index at %1 is damaged: %2\n"
                                        "Baloo is stopping. What happens on the next start depends on whether this has "
                                        "happened before: the index is built anew the first time, and left alone if the "
                                        "damage came back after a rebuild.\n"
                                        "Please report this at https://bugs.kde.org, and say what the machine was doing "
                                        "when it happened, so that the cause can be found.")
                                        .arg(path, reason);
}

void Database::lmdbAssertFailed(MDB_env *env, const char *message)
{
    QString path;
    if (auto *self = static_cast<Database *>(mdb_env_get_userctx(env))) {
        path = self->m_path;
    }
    noteCorruption(path, QString::fromUtf8(message));

    // The environment is left in an undefined state and may still hold the write lock,
    // so there is nothing left to do in this process. Leave without running the exit
    // handlers, which would touch the very state that is broken, and without the core
    // dump an abort would produce, since the message above says more than a backtrace
    // of the reader that happened to trip over the damage.
    _exit(EXIT_FAILURE);
}

Database::OpenResult Database::open(OpenMode mode)
{
    QMutexLocker locker(&m_mutex);

    // nop if already open!
    if (m_env) {
        if (mode == ReadOnlyDatabase) {
            return OpenResult::Success;
        } else if (m_mode == ReadWriteDatabase) {
            return OpenResult::Success;
        } else {
            return OpenResult::OpenedReadOnly;
        }
    }

    MDB_env* env = nullptr;

    QDir dir(m_path);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
        dir.refresh();
    }
    QFileInfo indexInfo(dir, QStringLiteral("index"));

    const CorruptionRecord corruption = readCorruptionRecord(m_path);
    if (corruption.count == 1 && indexInfo.exists()) {
        if (mode == CreateDatabase) {
            qCWarning(ENGINE).noquote() << QStringLiteral(
                                               "The index at %1 was damaged. Throwing it away and building a new one, "
                                               "which takes as long as the first indexing did.")
                                               .arg(m_path);
            QFile::remove(indexInfo.absoluteFilePath());
            QFile::remove(indexInfo.absoluteFilePath() + QStringLiteral("-lock"));
            writeCorruptionRecord(m_path, {corruption.count, QDateTime::currentDateTime()});
            indexInfo.refresh();
        } else {
            // The indexer owns the rebuild, everyone else stays out of the way until it has
            // happened.
            return OpenResult::InvalidDatabase;
        }
    } else if (corruption.count > 1) {
        qCCritical(ENGINE).noquote() << QStringLiteral(
                                            "The index at %1 was damaged again after it had been built anew, so something "
                                            "keeps breaking it and building it a third time would only repeat this.\n"
                                            "Baloo indexes nothing until you run 'balooctl6 purge', which throws the index "
                                            "away and starts over.\n"
                                            "Please report this at https://bugs.kde.org first, and say what the machine was "
                                            "doing when it happened, so that the cause can be found.")
                                            .arg(m_path);
        return OpenResult::InvalidDatabase;
    }

    if ((mode != CreateDatabase) && !indexInfo.exists()) {
        return OpenResult::InvalidPath;
    }

    if (mode == CreateDatabase) {
        if (!QFileInfo(dir.absolutePath()).permission(QFile::WriteOwner)) {
            qCCritical(ENGINE) << m_path << "does not have write permissions. Aborting";
            return OpenResult::InvalidPath;
        }

        if (!indexInfo.exists()) {
            FSUtils::disableCoW(m_path);
        }
    }

    int rc = mdb_env_create(&env);
    if (rc) {
        return OpenResult::InternalError;
    }

    // LMDB calls this when it walks into damage it cannot make sense of. Left to itself
    // it would abort the process with nothing said about what happened or what to do.
    mdb_env_set_userctx(env, this);
    mdb_env_set_assert(env, &Database::lmdbAssertFailed);

    /**
     * maximal number of allowed named databases, must match number of databases we create below
     * each additional one leads to overhead
     */
    mdb_env_set_maxdbs(env, 12);

    /**
     * size limit for database == size limit of mmap
     * use 1 GB on 32-bit, use 256 GB on 64-bit
     * Valgrind by default (without recompiling) limits the mmap size:
     * <= 3.9: 32 GByte, 3.9 to 3.12: 64 GByte, 3.13: 128 GByte
     */
    size_t sizeInGByte = 256;
    if (sizeof(void*) == 4) {
        sizeInGByte = 1;
        qCWarning(ENGINE) << "Running on 32 bit arch, limiting DB mmap to" << sizeInGByte << "GByte";
    } else if (RUNNING_ON_VALGRIND) {
        // valgrind lacks a runtime version check, assume valgrind >= 3.9, and allow for some other mmaps
        sizeInGByte = 40;
        qCWarning(ENGINE) << "Valgrind detected, limiting DB mmap to" << sizeInGByte << "GByte";
    }
    const size_t maximalSizeInBytes = sizeInGByte * size_t(1024) * size_t(1024) * size_t(1024);
    mdb_env_set_mapsize(env, maximalSizeInBytes);

    // Set MDB environment flags
    auto mdbEnvFlags = MDB_NOSUBDIR | MDB_NOMEMINIT;
    if (mode == ReadOnlyDatabase) {
        mdbEnvFlags |= MDB_RDONLY;
    }

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(indexInfo.absoluteFilePath());
    rc = mdb_env_open(env, arr.constData(), mdbEnvFlags, 0664);
    if (rc) {
        // mdb_env_close must be called when mdb_env_open fails
        mdb_env_close(env);
        if ((rc == ENOENT) || (rc == EACCES)) {
            return OpenResult::InvalidPath;
        }
        // Damage in the pages LMDB reads to open the file comes back as an error rather
        // than through the assert handler, so say the same thing here.
        if ((rc == MDB_CORRUPTED) || (rc == MDB_PANIC) || (rc == MDB_INVALID) || (rc == MDB_VERSION_MISMATCH)) {
            noteCorruption(m_path, QString::fromUtf8(mdb_strerror(rc)));
            return OpenResult::InvalidDatabase;
        }
        return OpenResult::InternalError;
    }

    rc = mdb_reader_check(env, nullptr);

    if (rc) {
        qCWarning(ENGINE) << "Database::open reader_check" << mdb_strerror(rc);
        mdb_env_close(env);
        return OpenResult::InternalError;
    }

    //
    // Individual Databases
    //
    MDB_txn* txn;
    if (mode != CreateDatabase) {
        int rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction ro begin" << mdb_strerror(rc);
            mdb_env_close(env);
            return OpenResult::InternalError;
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

        if (!m_dbis.isValid()) {
            qCWarning(ENGINE) << "dbis is invalid";
            mdb_txn_abort(txn);
            mdb_env_close(env);
            return OpenResult::InvalidDatabase;
        }

        rc = mdb_txn_commit(txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction ro commit" << mdb_strerror(rc);
            mdb_env_close(env);
            return OpenResult::InternalError;
        }
    } else {
        int rc = mdb_txn_begin(env, nullptr, 0, &txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction begin" << mdb_strerror(rc);
            mdb_env_close(env);
            return OpenResult::InternalError;
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

        if (!m_dbis.isValid()) {
            qCWarning(ENGINE) << "dbis is invalid";
            mdb_txn_abort(txn);
            mdb_env_close(env);
            return OpenResult::InvalidDatabase;
        }

        rc = mdb_txn_commit(txn);
        if (rc) {
            qCWarning(ENGINE) << "Database::transaction commit" << mdb_strerror(rc);
            mdb_env_close(env);
            return OpenResult::InternalError;
        }
    }

    Q_ASSERT(env);
    m_env = env;
    m_mode = (mode == ReadOnlyDatabase) ? ReadOnlyDatabase : ReadWriteDatabase;
    return OpenResult::Success;
}
