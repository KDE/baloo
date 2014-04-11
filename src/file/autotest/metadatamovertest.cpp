/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "metadatamovertest.h"
#include "../metadatamover.h"
#include "../lib/filemapping.h"

#include <QSignalSpy>
#include <QSqlQuery>

#include <QTest>
#include <QTimer>
#include <QDir>

using namespace Baloo;

MetadataMoverTest::MetadataMoverTest(QObject* parent)
    : QObject(parent)
    , m_db(0)
    , m_tempDir(0)
{
}

void MetadataMoverTest::init()
{

    m_tempDir = new KTempDir();
    m_db = new Database(this);
    m_db->setPath(m_tempDir->name());
    m_db->init();
}

void MetadataMoverTest::cleanupTestCase()
{
    delete m_db;
    m_db = 0;

    delete m_tempDir;
    m_tempDir = 0;
}

uint MetadataMoverTest::insertUrl(const QString& url)
{
    FileMapping file(url);
    file.create(m_db->sqlDatabase());

    return file.id();
}

void MetadataMoverTest::testRemoveFile()
{
    const QString url("/home/vishesh/t");
    uint fid = insertUrl(url);

    MetadataMover mover(m_db, this);

    QSignalSpy spy(&mover, SIGNAL(fileRemoved(int)));
    mover.removeFileMetadata(url);

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().toUInt(), fid);

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("select count(*) from files;");
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

void MetadataMoverTest::testMoveFile()
{
    const QString url("/home/vishesh/t");
    uint fid = insertUrl(url);

    MetadataMover mover(m_db, this);

    const QString newUrl("/home/vishesh/p");
    mover.moveFileMetadata(url, newUrl);

    QEventLoop loop;
    QTimer::singleShot(5, &loop, SLOT(quit()));
    loop.exec();

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("select count(*) from files;");
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);
    QVERIFY(!query.next());

    query.prepare("select url from files where id = ?;");
    query.addBindValue(fid);

    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), newUrl);
    QVERIFY(!query.next());
}

void MetadataMoverTest::testMoveFolder()
{
    const QString folderUrl(m_tempDir->name() + "folder");
    uint folId = insertUrl(folderUrl);

    // The directory needs to be created because moveFileMetadata checks if it is
    // a directory in order to do the more complicated sql query to rename the
    // files in that folder
    QDir dir(m_tempDir->name());
    dir.mkdir("folder");
    QVERIFY(QDir(folderUrl).exists());

    const QString fileUrl1(folderUrl + "/1");
    uint fid1 = insertUrl(fileUrl1);

    const QString fileUrl2(folderUrl + "/2");
    uint fid2 = insertUrl(fileUrl2);

    MetadataMover mover(m_db, this);

    const QString newFolderUrl(m_tempDir->name() + "p");
    dir.rename("folder", "p");
    mover.moveFileMetadata(folderUrl, newFolderUrl);

    QEventLoop loop;
    QTimer::singleShot(5, &loop, SLOT(quit()));
    loop.exec();

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("select count(*) from files;");
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 3);
    QVERIFY(!query.next());

    query.prepare("select url from files where id = ?;");
    query.addBindValue(folId);

    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), newFolderUrl);
    QVERIFY(!query.next());

    query.prepare("select url from files where id = ?;");
    query.addBindValue(fid1);

    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), newFolderUrl + "/1");
    QVERIFY(!query.next());

    query.prepare("select url from files where id = ?;");
    query.addBindValue(fid2);

    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), newFolderUrl + "/2");
    QVERIFY(!query.next());
}

QTEST_MAIN(MetadataMoverTest)
