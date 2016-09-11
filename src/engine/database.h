/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 * Copyright (C) 2016  Christoph Cullmann <cullmann@kde.org>
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
         * Create + open read-write dabase.
         */
        CreateDatabase,

        /**
         * Read-Write Database, only works if database exists.
         */
        ReadWriteDatabase,

        /**
         * Read-Only Database, only works if database exists.
         */
        ReadOnlyDatabase
    };

    /**
     * Open database in given mode.
     * Nop after open was done (even if mode differs).
     * There is no close as this would invalidate the database for all threads using it.
     * @param mode create or open only?
     * @return success?
     */
    bool open(OpenMode mode);

    /**
     * Is database open?
     * @return database open?
     */
    bool isOpen() const;

    /**
     * Path to database.
     * @return database path
     */
    QString path() const;

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

    friend class Transaction;
    friend class DatabaseTest;

};
}

#endif // BALOO_DATABASE_H
