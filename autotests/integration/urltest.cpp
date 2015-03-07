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

#include "documenturldb.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QDebug>

using namespace Baloo;

class Baloo::UrlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        m_tempDir = new QTemporaryDir();

        mdb_env_create(&m_env);
        mdb_env_set_maxdbs(m_env, 2);

        // The directory needs to be created before opening the environment
        QByteArray path = QFile::encodeName(m_tempDir->path());
        mdb_env_open(m_env, path.constData(), 0, 0664);
        mdb_txn_begin(m_env, NULL, 0, &m_txn);
    }

    void cleanup() {
        mdb_txn_abort(m_txn);
        mdb_env_close(m_env);
        delete m_tempDir;
    }

    void test();
private:
    MDB_env* m_env;
    MDB_txn* m_txn;
    QTemporaryDir* m_tempDir;
};

static bool touch(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }

    QTextStream s(&f);
    s << "bah";

    return true;
}

static quint64 idForPath(const QString& filePath)
{
    QT_STATBUF statBuf;
    QT_LSTAT(QFile::encodeName(filePath).constData(), &statBuf);

    return statBufToId(statBuf);
}

void UrlTest::test()
{
    QTemporaryDir dir;

    QString filePath = dir.path() + "file";
    QVERIFY(touch(filePath));
    quint64 id = idForPath(filePath);

    DocumentUrlDB db(m_txn);
    QByteArray arr = QFile::encodeName(filePath);
    db.put(id, arr);
    QCOMPARE(db.get(id), arr);

    // Need to check if a proper tree was created in the IdTree
    // Need to check if the IdFilenameDB was properly updated!
    // How?
}

QTEST_MAIN(UrlTest)

#include "urltest.moc"
