/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014  Vishesh Handa <vhanda@kde.org>
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

#include "metadatamover.h"

#include "database.h"
#include "document.h"
#include "basicindexingjob.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QDir>
#include <qtemporaryfile.h>

using namespace Baloo;

class MetadataMoverTest : public QObject
{
    Q_OBJECT
public:
    MetadataMoverTest(QObject* parent = 0);

private Q_SLOTS:

    void init();
    void cleanupTestCase();

    void testRemoveFile();
    void testMoveFile();
    void testMoveFolder();

private:
    quint64 insertUrl(const QString& url);

    Database* m_db;
    QTemporaryDir* m_tempDir;
};

MetadataMoverTest::MetadataMoverTest(QObject* parent)
    : QObject(parent)
    , m_db(0)
    , m_tempDir(0)
{
}

void MetadataMoverTest::init()
{
    m_tempDir = new QTemporaryDir();
    m_db = new Database(m_tempDir->path());
    m_db->open();
    m_db->transaction(Database::ReadWrite);
}

void MetadataMoverTest::cleanupTestCase()
{
    delete m_db;
    m_db = 0;

    delete m_tempDir;
    m_tempDir = 0;
}

quint64 MetadataMoverTest::insertUrl(const QString& url)
{
    BasicIndexingJob job(url, QStringLiteral("text/plain"), false);
    job.index();

    m_db->addDocument(job.document());
    return job.document().id();
}

void MetadataMoverTest::testRemoveFile()
{
    QTemporaryFile file;
    file.open();
    QString url = file.fileName();
    quint64 fid = insertUrl(url);

    m_db->commit();
    m_db->transaction(Database::ReadWrite);

    QVERIFY(m_db->hasDocument(fid));
    MetadataMover mover(m_db, this);
    file.remove();
    mover.removeFileMetadata(url);

    QVERIFY(!m_db->hasDocument(fid));
}

void MetadataMoverTest::testMoveFile()
{
    /*
    const QString url(QLatin1String("/home/vishesh/t"));
    quint64 fid = insertUrl(url);

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
    QByteArray arr = "P-" + newUrl.toUtf8();
    QCOMPARE(doc.fetchTermStartsWith("P"), QString::fromUtf8(arr));
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);
    */
}

void MetadataMoverTest::testMoveFolder()
{
    /*
    const QString folderUrl(m_tempDir->path() + QLatin1String("/folder"));
    quint64 folId = insertUrl(folderUrl);

    // The directory needs to be created because moveFileMetadata checks if it is
    // a directory in order to do the more complicated query to rename the
    // files in that folder
    QDir dir(m_tempDir->path());
    dir.mkdir(QLatin1String("folder"));
    QVERIFY(QDir(folderUrl).exists());

    const QString fileUrl1(folderUrl + QLatin1String("/1"));
    quint64 fid1 = insertUrl(fileUrl1);

    const QString fileUrl2(folderUrl + QLatin1String("/2"));
    quint64 fid2 = insertUrl(fileUrl2);

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
    QByteArray arr = "P-" + newFolderUrl.toUtf8();
    QCOMPARE(doc.fetchTermStartsWith("P"), QString::fromUtf8(arr));
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);

    // File 1
    doc = m_db->xapianDatabase()->document(fid1);
    QString str = newFolderUrl + "/1";
    QCOMPARE(QString::fromUtf8(doc.value(3)), str);
    arr = "P-" + str.toUtf8();
    QCOMPARE(doc.fetchTermStartsWith("P"), QString::fromUtf8(arr));
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);

    // File 2
    doc = m_db->xapianDatabase()->document(fid2);
    str = newFolderUrl + "/2";
    QCOMPARE(QString::fromUtf8(doc.value(3)), str);
    arr = "P-" + str.toUtf8();
    QCOMPARE(doc.fetchTermStartsWith("P"), QString::fromUtf8(arr));
    QCOMPARE(doc.fetchTermsStartsWith("P").size(), 1);
    */
}

QTEST_MAIN(MetadataMoverTest)

#include "metadatamovertest.moc"
