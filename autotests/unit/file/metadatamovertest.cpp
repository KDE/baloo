/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "metadatamover.h"

#include "database.h"
#include "transaction.h"
#include "document.h"
#include "basicindexingjob.h"

#include <memory>
#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

using namespace Baloo;

class MetadataMoverTest : public QObject
{
    Q_OBJECT
public:
    MetadataMoverTest(QObject* parent = nullptr);

private Q_SLOTS:

    void init();

    void testRemoveFile();
    void testRenameFile();
    void testMoveFile();
    void testMoveRenameFile();
    void testMoveFolder();
    void testMoveTwiceSequential();
    void testMoveTwiceCombined();
    void testMoveDeleteFile();
    void testMoveFileRenameParent();
    void testRenameMoveFileRenameParent();
    void testMoveFileMoveParent();

private:
    quint64 insertUrl(const QString& url);

    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<Database> m_db;

    struct DocumentInfo {
        QString url;
        DocumentTimeDB::TimeInfo timeInfo;
        QVector<QByteArray> docTerms;
        QVector<QByteArray> filenameTerms;
    };
    DocumentInfo documentInfo(quint64 docId);
};

MetadataMoverTest::MetadataMoverTest(QObject* parent)
    : QObject(parent)
{
}

void MetadataMoverTest::init()
{
    m_tempDir = std::make_unique<QTemporaryDir>();
    m_db = std::make_unique<Database>(m_tempDir->path());
    m_db->open(Database::CreateDatabase);
    QVERIFY(m_db->isOpen());
}

quint64 MetadataMoverTest::insertUrl(const QString& url)
{
    BasicIndexingJob job(url, QStringLiteral("text/plain"));
    job.index();

    Transaction tr(m_db.get(), Transaction::ReadWrite);
    tr.addDocument(job.document());
    tr.commit();
    return job.document().id();
}

MetadataMoverTest::DocumentInfo MetadataMoverTest::documentInfo(quint64 id)
{
    Transaction tr(m_db.get(), Transaction::ReadOnly);
    if (!tr.hasDocument(id)) {
        return DocumentInfo();
    }

    DocumentInfo docInfo;
    docInfo.url = QFile::decodeName(tr.documentUrl(id));
    docInfo.timeInfo = tr.documentTimeInfo(id);
    docInfo.docTerms = tr.documentTerms(id);
    docInfo.filenameTerms = tr.documentFileNameTerms(id);

    return docInfo;
}

void MetadataMoverTest::testRemoveFile()
{
    QTemporaryFile file;
    file.open();
    QString url = file.fileName();
    quint64 fid = insertUrl(url);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db.get(), this);
    file.remove();
    mover.removeFileMetadata(url);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
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

    QString url = dir.path() + QStringLiteral("/file");
    touchFile(url);
    quint64 fid = insertUrl(url);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file2");
    QFile::rename(url, url2);
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QVERIFY(newInfo.filenameTerms != oldInfo.filenameTerms);
    QVERIFY(!newInfo.filenameTerms.empty());
}

void MetadataMoverTest::testMoveFile()
{
    QTemporaryDir dir;
    QDir().mkpath(dir.path() + QStringLiteral("/a/b/c"));

    QString url = dir.path() + QStringLiteral("/a/b/c/file");
    touchFile(url);
    quint64 fid = insertUrl(url);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file");
    QFile::rename(url, url2);
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QCOMPARE(newInfo.filenameTerms, oldInfo.filenameTerms);
}

void MetadataMoverTest::testMoveRenameFile()
{
    QTemporaryDir dir;
    QDir().mkpath(dir.path() + QStringLiteral("/a/b/c"));

    QString url = dir.path() + QStringLiteral("/a/b/c/file");
    touchFile(url);
    quint64 fid = insertUrl(url);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file2");
    QFile::rename(url, url2);
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QVERIFY(newInfo.filenameTerms != oldInfo.filenameTerms);
    QVERIFY(!newInfo.filenameTerms.empty());
}

void MetadataMoverTest::testMoveFolder()
{
    QTemporaryDir dir;

    QString folder = dir.path() + QStringLiteral("/folder");
    mkdir(folder);
    quint64 did = insertUrl(folder);

    QString fileUrl = folder + QStringLiteral("/file");
    touchFile(fileUrl);
    quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    QString newFolderUrl = dir.path() + QStringLiteral("/dir");
    QFile::rename(folder, newFolderUrl);
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(folder, newFolderUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(newFolderUrl));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(newFolderUrl) + "/file");
    }
}

// Rename a file twice in a row
// Mimic Inotify/Filewatch behavior as if there was sufficient
// time to process the first rename before the second happens
void MetadataMoverTest::testMoveTwiceSequential()
{
    QTemporaryDir dir;
    const quint64 did = insertUrl(dir.path());

    const QString fileUrl = dir.path() + QStringLiteral("/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/file1");
    QFile::rename(fileUrl, fileUrl1);

    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);

    // Second rename
    const QString fileUrl2 = dir.path() + QStringLiteral("/file2");
    QFile::rename(fileUrl1, fileUrl2);

    mover.moveFileMetadata(fileUrl1, fileUrl2);

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
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

    const QString fileUrl = dir.path() + QStringLiteral("/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/file1");
    QFile::rename(fileUrl, fileUrl1);

    // Second rename
    const QString fileUrl2 = dir.path() + QStringLiteral("/file2");
    QFile::rename(fileUrl1, fileUrl2);

    // "Flush" rename notificatons
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);
    mover.moveFileMetadata(fileUrl1, fileUrl2);

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
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

    const QString fileUrl = dir.path() + QStringLiteral("/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/file1");
    QFile::rename(fileUrl, fileUrl1);

    QFile::remove(fileUrl1);

    // "Flush" notificatons
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);
    mover.removeFileMetadata(fileUrl1);

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
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

    QDir().mkpath(dir.path() + QStringLiteral("/a"));
    quint64 did_a = insertUrl(dir.path() + QStringLiteral("/a"));
    QDir().mkpath(dir.path() + QStringLiteral("/b"));
    quint64 did_b = insertUrl(dir.path() + QStringLiteral("/b"));

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + QStringLiteral("/b/file1");
    QVERIFY(QFile::rename(fileUrl, fileUrl_b1));

    // Rename parent dir (replacing old "a" dir)
    QVERIFY(QDir::current().rmdir(dir.path() + QStringLiteral("/a")));
    QVERIFY(QFile::rename(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a")));

    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/a")));
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/a/file1")));
    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/b")));

    // "Flush" notificatons
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl_b1);
    mover.removeFileMetadata(dir.path() + QStringLiteral("/a"));
    mover.moveFileMetadata(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path()) + "/a");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/a/file1");
    }
}

// Rename a file, move to a different parent directory and rename the new parent
void MetadataMoverTest::testRenameMoveFileRenameParent()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    QDir().mkpath(dir.path() + QStringLiteral("/a"));
    quint64 did_a = insertUrl(dir.path() + QStringLiteral("/a"));
    QDir().mkpath(dir.path() + QStringLiteral("/b"));
    quint64 did_b = insertUrl(dir.path() + QStringLiteral("/b"));

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl_a1 = dir.path() + QStringLiteral("/a/file1");
    QFile::rename(fileUrl, fileUrl_a1);

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + QStringLiteral("/b/file1");
    QVERIFY(QFile::rename(fileUrl_a1, fileUrl_b1));

    // Rename parent dir (replacing old "a" dir)
    QVERIFY(QDir::current().rmdir(dir.path() + QStringLiteral("/a")));
    QVERIFY(QFile::rename(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a")));

    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/a")));
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/a/file1")));
    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/b")));

    // "Flush" notificatons
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl_a1);
    mover.moveFileMetadata(fileUrl_a1, fileUrl_b1);
    mover.removeFileMetadata(dir.path() + QStringLiteral("/a"));
    mover.moveFileMetadata(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path()) + "/a");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/a/file1");
    }
}

// Rename a file and then immediately rename a parent directory
void MetadataMoverTest::testMoveFileMoveParent()
{
    QTemporaryDir dir;
    quint64 did = insertUrl(dir.path());

    QDir().mkpath(dir.path() + QStringLiteral("/a"));
    quint64 did_a = insertUrl(dir.path() + QStringLiteral("/a"));

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    touchFile(fileUrl);
    const quint64 fid = insertUrl(fileUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/a/file1");
    QFile::rename(fileUrl, fileUrl1);

    // Rename parent dir
    QFile::rename(dir.path() + QStringLiteral("/a"), dir.path() + QStringLiteral("/b"));

    // "Flush" notificatons
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);
    mover.moveFileMetadata(dir.path() + QStringLiteral("/a"), dir.path() + QStringLiteral("/b"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_a), QFile::encodeName(dir.path()) + "/b");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/b/file1");
    }
}

QTEST_GUILESS_MAIN(MetadataMoverTest)

#include "metadatamovertest.moc"
