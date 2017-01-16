/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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
#include "transaction.h"
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
    MetadataMoverTest(QObject* parent = nullptr);

private Q_SLOTS:

    void init();
    void cleanupTestCase();

    void testRemoveFile();
    void testRenameFile();
    void testMoveFile();
    void testMoveFolder();

private:
    quint64 insertUrl(const QString& url);

    Database* m_db;
    QTemporaryDir* m_tempDir;
};

MetadataMoverTest::MetadataMoverTest(QObject* parent)
    : QObject(parent)
    , m_db(nullptr)
    , m_tempDir(nullptr)
{
}

void MetadataMoverTest::init()
{
    m_tempDir = new QTemporaryDir();
    m_db = new Database(m_tempDir->path());
    m_db->open(Database::CreateDatabase);
}

void MetadataMoverTest::cleanupTestCase()
{
    delete m_db;
    m_db = nullptr;

    delete m_tempDir;
    m_tempDir = nullptr;
}

quint64 MetadataMoverTest::insertUrl(const QString& url)
{
    BasicIndexingJob job(url, QStringLiteral("text/plain"));
    job.index();

    Transaction tr(m_db, Transaction::ReadWrite);
    tr.addDocument(job.document());
    tr.commit();
    return job.document().id();
}

void MetadataMoverTest::testRemoveFile()
{
    QTemporaryFile file;
    file.open();
    QString url = file.fileName();
    quint64 fid = insertUrl(url);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db, this);
    file.remove();
    mover.removeFileMetadata(url);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(!tr.hasDocument(fid));
    }
}

static void touchFile(const QString& path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    QTextStream s(&file);
    s << "random";
}
static void mkdir(const QString& path)
{
    QDir().mkpath(path);
    QVERIFY(QDir(path).exists());
}

void MetadataMoverTest::testRenameFile()
{
    QTemporaryDir dir;

    QString url = dir.path() + "/file";
    touchFile(url);
    quint64 fid = insertUrl(url);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db, this);
    QString url2 = dir.path() + "/file2";
    QFile::rename(url, url2);
    mover.moveFileMetadata(QFile::encodeName(url), QFile::encodeName(url2));

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(url2));
    }
}

void MetadataMoverTest::testMoveFile()
{
    QTemporaryDir dir;
    QDir().mkpath(dir.path() + "/a/b/c");

    QString url = dir.path() + "/a/b/c/file";
    touchFile(url);
    quint64 fid = insertUrl(url);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db, this);
    QString url2 = dir.path() + "/file2";
    QFile::rename(url, url2);
    mover.moveFileMetadata(QFile::encodeName(url), QFile::encodeName(url2));

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(url2));
    }
}

void MetadataMoverTest::testMoveFolder()
{
    QTemporaryDir dir;

    QString folder = dir.path() + "/folder";
    mkdir(folder);
    quint64 did = insertUrl(folder);

    QString fileUrl = folder + "/file";
    touchFile(fileUrl);
    quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    QString newFolderUrl = dir.path() + "/dir";
    QFile::rename(folder, newFolderUrl);
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(folder), QFile::encodeName(newFolderUrl));

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(newFolderUrl));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(newFolderUrl + "/file"));
    }
}

QTEST_GUILESS_MAIN(MetadataMoverTest)

#include "metadatamovertest.moc"
