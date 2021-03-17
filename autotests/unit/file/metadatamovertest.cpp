/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "metadatamover.h"

#include "basicindexingjob.h"
#include "database.h"
#include "document.h"
#include "transaction.h"

#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
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
    void testMoveTwiceSequential();
    void testMoveTwiceCombined();
    void testMoveDeleteFile();
    void testMoveFileRenameParent();
    void testRenameMoveFileRenameParent();
    void testMoveFileMoveParent();

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

// Rename a file twice in a row
// Mimic Inotify/Filewatch behavior as if there was sufficient
// time to process the first rename before the second happens
void MetadataMoverTest::testMoveTwiceSequential()
{
    QTemporaryDir dir;
    const quint64 did = insertUrl(dir.path());

    const QString fileUrl = dir.path() + "/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + "/file1";
    QFile::rename(fileUrl, fileUrl1);

    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl1));

    // Second rename
    const QString fileUrl2 = dir.path() + "/file2";
    QFile::rename(fileUrl1, fileUrl2);

    mover.moveFileMetadata(QFile::encodeName(fileUrl1), QFile::encodeName(fileUrl2));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(fileUrl2));
    }
}

// Rename a file twice in a row
// Similar to `testMoveTwiceSequential`, but this time the
// second rename happens before the first one is processed
void MetadataMoverTest::testMoveTwiceCombined()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    const QString fileUrl = dir.path() + "/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + "/file1";
    QFile::rename(fileUrl, fileUrl1);

    // Second rename
    const QString fileUrl2 = dir.path() + "/file2";
    QFile::rename(fileUrl1, fileUrl2);

    // "Flush" rename notificatons
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl1));
    mover.moveFileMetadata(QFile::encodeName(fileUrl1), QFile::encodeName(fileUrl2));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(fileUrl2));
    }
}

// Rename a file and then immediately delete it
void MetadataMoverTest::testMoveDeleteFile()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    const QString fileUrl = dir.path() + "/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + "/file1";
    QFile::rename(fileUrl, fileUrl1);

    QFile::remove(fileUrl1);

    // "Flush" notificatons
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl1));
    mover.removeFileMetadata(QFile::encodeName(fileUrl1));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QVERIFY(!tr.hasDocument(fid));
    }
}

// Move a file to a different parent directory and rename the new parent
void MetadataMoverTest::testMoveFileRenameParent()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    QDir().mkpath(dir.path() + "/a");
    quint64 did_a = insertUrl(dir.path() + "/a");
    QDir().mkpath(dir.path() + "/b");
    quint64 did_b = insertUrl(dir.path() + "/b");

    const QString fileUrl = dir.path() + "/a/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + "/b/file1";
    QVERIFY(QFile::rename(fileUrl, fileUrl_b1));

    // Rename parent dir (replacing old "a" dir)
    QVERIFY(QDir::current().rmdir(dir.path() + "/a"));
    QVERIFY(QFile::rename(dir.path() + "/b", dir.path() + "/a"));

    QVERIFY(QFile::exists(dir.path() + "/a"));
    QVERIFY(QFile::exists(dir.path() + "/a/file1"));
    QVERIFY(!QFile::exists(dir.path() + "/b"));

    // "Flush" notificatons
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl_b1));
    mover.removeFileMetadata(QFile::encodeName(dir.path() + "/a"));
    mover.moveFileMetadata(QFile::encodeName(dir.path() + "/b"), QFile::encodeName(dir.path() + "/a"));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path() + "/a"));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path() + "/a/file1"));
    }
}

// Rename a file, move to a different parent directory and rename the new parent
void MetadataMoverTest::testRenameMoveFileRenameParent()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    QDir().mkpath(dir.path() + "/a");
    quint64 did_a = insertUrl(dir.path() + "/a");
    QDir().mkpath(dir.path() + "/b");
    quint64 did_b = insertUrl(dir.path() + "/b");

    const QString fileUrl = dir.path() + "/a/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl_a1 = dir.path() + "/a/file1";
    QFile::rename(fileUrl, fileUrl_a1);

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + "/b/file1";
    QVERIFY(QFile::rename(fileUrl_a1, fileUrl_b1));

    // Rename parent dir (replacing old "a" dir)
    QVERIFY(QDir::current().rmdir(dir.path() + "/a"));
    QVERIFY(QFile::rename(dir.path() + "/b", dir.path() + "/a"));

    QVERIFY(QFile::exists(dir.path() + "/a"));
    QVERIFY(QFile::exists(dir.path() + "/a/file1"));
    QVERIFY(!QFile::exists(dir.path() + "/b"));

    // "Flush" notificatons
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl_a1));
    mover.moveFileMetadata(QFile::encodeName(fileUrl_a1), QFile::encodeName(fileUrl_b1));
    mover.removeFileMetadata(QFile::encodeName(dir.path() + "/a"));
    mover.moveFileMetadata(QFile::encodeName(dir.path() + "/b"), QFile::encodeName(dir.path() + "/a"));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path() + "/a"));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path() + "/a/file1"));
    }
}

// Rename a file and then immediately rename a parent directory
void MetadataMoverTest::testMoveFileMoveParent()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    QDir().mkpath(dir.path() + "/a");
    quint64 did_a = insertUrl(dir.path() + "/a");

    const QString fileUrl = dir.path() + "/a/file";
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + "/a/file1";
    QFile::rename(fileUrl, fileUrl1);

    // Rename parent dir
    QFile::rename(dir.path() + "/a", dir.path() + "/b");

    // "Flush" notificatons
    MetadataMover mover(m_db, this);
    mover.moveFileMetadata(QFile::encodeName(fileUrl), QFile::encodeName(fileUrl1));
    mover.moveFileMetadata(QFile::encodeName(dir.path() + "/a"), QFile::encodeName(dir.path() + "/b"));

    // Check result
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_a), QFile::encodeName(dir.path() + "/b"));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path() + "/b/file1"));
    }
}

QTEST_GUILESS_MAIN(MetadataMoverTest)

#include "metadatamovertest.moc"
