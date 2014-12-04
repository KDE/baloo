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
#include "filemapping.h"

#include "xapiandatabase.h"
#include "xapiandocument.h"

#include <QSignalSpy>

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
    m_tempDir = new QTemporaryDir();
    m_db = new Database(this);
    m_db->setPath(m_tempDir->path());
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
    XapianDocument doc;
    doc.addValue(3, url);
    doc.addBoolTerm(url, "P");

    m_db->xapianDatabase()->addDocument(doc);
    m_db->xapianDatabase()->commit();

    XapianDocument doc2 = m_db->xapianDatabase()->document(1);
    Xapian::Enquire enquire(*m_db->xapianDatabase()->db());
    enquire.set_query(Xapian::Query(("P" + url).toUtf8().constData()));
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    Xapian::MSet mset = enquire.get_mset(0, 1);
    Xapian::MSetIterator miter = mset.begin();
    Q_ASSERT(miter != mset.end());

    return *miter;
}

void MetadataMoverTest::testRemoveFile()
{
    const QString url(QLatin1String("/home/vishesh/t"));
    uint fid = insertUrl(url);

    MetadataMover mover(m_db, this);

    QSignalSpy spy(&mover, SIGNAL(fileRemoved(int)));
    mover.removeFileMetadata(url);

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().toUInt(), fid);

    Xapian::Database* db = m_db->xapianDatabase()->db();
    QVERIFY(db->get_doccount() == 0);
}

void MetadataMoverTest::testMoveFile()
{
    const QString url(QLatin1String("/home/vishesh/t"));
    uint fid = insertUrl(url);

    MetadataMover mover(m_db, this);

    const QString newUrl(QLatin1String("/home/vishesh/p"));
    mover.moveFileMetadata(url, newUrl);

    QEventLoop loop;
    QTimer::singleShot(5, &loop, SLOT(quit()));
    loop.exec();

    Xapian::Database* db = m_db->xapianDatabase()->db();
    QVERIFY(db->get_doccount() == 1);

    XapianDocument doc = m_db->xapianDatabase()->document(fid);
    QCOMPARE(QString::fromUtf8(doc.value(3)), newUrl);
    QString str = "P" + newUrl;
    QCOMPARE(doc.fetchTermStartsWith("P"), str);
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);
}

void MetadataMoverTest::testMoveFolder()
{
    const QString folderUrl(m_tempDir->path() + QLatin1String("/folder"));
    uint folId = insertUrl(folderUrl);

    // The directory needs to be created because moveFileMetadata checks if it is
    // a directory in order to do the more complicated sql query to rename the
    // files in that folder
    QDir dir(m_tempDir->path());
    dir.mkdir(QLatin1String("folder"));
    QVERIFY(QDir(folderUrl).exists());

    const QString fileUrl1(folderUrl + QLatin1String("/1"));
    uint fid1 = insertUrl(fileUrl1);

    const QString fileUrl2(folderUrl + QLatin1String("/2"));
    uint fid2 = insertUrl(fileUrl2);

    MetadataMover mover(m_db, this);

    const QString newFolderUrl(m_tempDir->path() + QLatin1String("/p"));
    dir.rename(QLatin1String("folder"), QLatin1String("p"));
    mover.moveFileMetadata(folderUrl, newFolderUrl);

    QEventLoop loop;
    QTimer::singleShot(5, &loop, SLOT(quit()));
    loop.exec();

    Xapian::Database* db = m_db->xapianDatabase()->db();
    QVERIFY(db->get_doccount() == 3);

    // Folder
    XapianDocument doc = m_db->xapianDatabase()->document(folId);
    QCOMPARE(QString::fromUtf8(doc.value(3)), newFolderUrl);
    QString str = "P" + newFolderUrl;
    QCOMPARE(doc.fetchTermStartsWith("P"), str);
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);

    // File 1
    doc = m_db->xapianDatabase()->document(fid1);
    str = newFolderUrl + "/1";
    QCOMPARE(QString::fromUtf8(doc.value(3)), str);
    str = "P" + str;
    QCOMPARE(doc.fetchTermStartsWith("P"), str);
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);

    // File 2
    doc = m_db->xapianDatabase()->document(fid2);
    str = newFolderUrl + "/2";
    QCOMPARE(QString::fromUtf8(doc.value(3)), str);
    str = "P" + str;
    QCOMPARE(doc.fetchTermStartsWith("P"), str);
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);
}

QTEST_MAIN(MetadataMoverTest)
