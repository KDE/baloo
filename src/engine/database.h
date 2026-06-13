/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DATABASE_H
#define BALOO_DATABASE_H

#include <QMutex>

#include "document.h"
#include "databasedbis.h"

namespace Baloo {

class DatabaseTest;

class BALOO_ENGINE_EXPORT Database
{
public:
    /**
     * Init database for given DB path, will not open it.
     * @param path db path
     */
    explicit Database(const QString& path);

    /**
     * Destruct db, might close it, if opened.
     */
    ~Database();

    /**
     * Database open mode
     */
    enum OpenMode {
        /**
         * Create + open read-write database.
         */
        CreateDatabase,

        /**
         * Read-Write Database, only works if database exists.
         */
        ReadWriteDatabase,

        /**
         * Read-Only Database, only works if database exists.
         */
        ReadOnlyDatabase,
    };

    enum class OpenResult {
        // clang-format off
        Success,
        InvalidPath,     ///< Database does not exist, or could not be created
        InvalidDatabase, ///< Database structure does not match expectation
        InternalError,   ///< Internal error in the database engine
        OpenedReadOnly,  ///< Database was opened in readonly mode
        // clang-format on
    };

    /**
     * Open database in given mode.
     * Nop after open was done (even if mode differs).
     * There is no close as this would invalidate the database for all threads using it.
     * @param mode create or open only?
     */
    OpenResult open(OpenMode mode);

private:
    /**
     * serialize access, as open might be called from multiple threads
     */
    mutable QMutex m_mutex;

    /**
     * database path
     */
    const QString m_path;

    MDB_env* m_env;
    DatabaseDbis m_dbis;
    /// m_mode is only valid if m_env is valid
    OpenMode m_mode = ReadOnlyDatabase;

    /// Marks the index as damaged, so that the next open knows to keep away from it.
    static QString corruptionMarkerPath(const QString &path);
    /// Leaves the mark and says what happened.
    static void noteCorruption(const QString &path, const QString &reason);
    /// Says what happened and what the user can do about it.
    static void reportCorruption(const QString &path, const QString &reason);

    /// LMDB assert callback: records the damage, then leaves the process.
    static void lmdbAssertFailed(MDB_env *env, const char *message);

    friend class Transaction;
    friend class DatabaseTest;

};
}

#endif // BALOO_DATABASE_H
