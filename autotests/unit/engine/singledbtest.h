/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
