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

#ifndef BALOO_SINGLEDBTEST_H
#define BALOO_SINGLEDBTEST_H

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

#include <lmdb.h>

class SingleDBTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_tempDir = new QTemporaryDir();

        mdb_env_create(&m_env);
        mdb_env_set_maxdbs(m_env, 1);

        // The directory needs to be created before opening the environment
        QByteArray path = QFile::encodeName(m_tempDir->path());
        mdb_env_open(m_env, path.constData(), 0, 0664);
        mdb_txn_begin(m_env, nullptr, 0, &m_txn);
    }

    void cleanup()
    {
        mdb_txn_abort(m_txn);
        mdb_env_close(m_env);
        delete m_tempDir;
    }

protected:
    MDB_env* m_env;
    MDB_txn* m_txn;
    QTemporaryDir* m_tempDir;
};

#endif // SINGLEDBTEST_H
