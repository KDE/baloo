/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "modifiedfileindexer.h"

#include "basicindexingjob.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "fileindexerconfigutils.h"
#include "idutils.h"
#include "transaction.h"

#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;
using namespace Qt::StringLiterals;

namespace
{
auto fileTestData()
{
    return std::array<std::tuple<QString, QByteArrayList>, 3>{{
        {u"first.txt"_s, {"Mtext", "Mplain"}},
        {u"second.pbm"_s, {"Mimage", "Mx", "Mportable", "Mbitmap"}},
        {u"_tempfile"_s, {"Mapplication", "Moctet", "Mstream"}},
    }};
}

void addFile(const QString &name, QByteArrayView data)
{
    QFile file(name);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(data.size(), file.write(data.data(), data.size()));
    file.close();
    QVERIFY(QFile::exists(name));
}

void adjustTime(const QString &name, QFile::FileTime fileTime, int adjust)
{
    QFile file(name);
    QVERIFY(file.open(QIODevice::ReadOnly));
    auto dateTime = file.fileTime(fileTime);
    dateTime += std::chrono::seconds(adjust);
    QVERIFY(file.setFileTime(dateTime, fileTime));
    file.close();
}
} // namespace <anonymous>

class ModifiedFileIndexerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        dir = std::make_unique<QTemporaryDir>();
        db = std::make_unique<Database>(dir->path());
        QCOMPARE(db->open(Database::CreateDatabase), Database::OpenResult::Success);

        QVERIFY(QDir(dir->path()).mkdir(u"files"_s));
        Test::writeIndexerConfig( //
            {dir->filePath(u"files"_s)}, // includeFolders
            {}, // excludeFolders
            {}, // excludeFilters
            false, // indexHidden
            true // onlyBasicIndexing
        );

        // Prime the DB
        BasicIndexingJob job(dir->filePath(u"files"_s), u"inode/directory"_s, BasicIndexingJob::NoLevel);
        QVERIFY(job.index());

        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(job.document());
        QVERIFY(tr.commit());

        filesDir = QDir{dir->filePath(u"files"_s)};
    }

    void cleanup()
    {
        db.reset();
        dir.reset();
    }

    void testBaseDir();
    void testAddInitialEmpty();
    void testAddInitialEmpty_data();
    void testAddInitialFilled();
    void testAddInitialFilled_data();
    void testModifyExisting();
    void testModifyExisting_data();
    void testRename();

private:
    std::unique_ptr<QTemporaryDir> dir;
    std::unique_ptr<Database> db;
    QDir filesDir;
};

void ModifiedFileIndexerTest::testBaseDir()
{
    Transaction tr(db.get(), Transaction::ReadOnly);

    quint64 filesDirId = filePathToId(QFile::encodeName(dir->filePath(u"files"_s)));
    QVERIFY(tr.hasDocument(filesDirId));

    const auto fullPath = tr.documentUrl(filesDirId);
    QCOMPARE(fullPath, QFile::encodeName(dir->filePath(u"files"_s)));

    QCOMPARE(tr.documentId(fullPath), filesDirId);
}

void ModifiedFileIndexerTest::testAddInitialEmpty_data()
{
    QTest::addColumn<QString>("filename");

    for (const auto &name : {
             u"first.txt"_s,
             u"second.pbm"_s,
             u"_tempfile"_s,
         }) {
        QTest::addRow("%s", qPrintable(name)) << name;
    }
}

void ModifiedFileIndexerTest::testAddInitialEmpty()
{
    QFETCH(QString, filename);

    const auto path{filesDir.filePath(filename)};
    addFile(path, "");

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(!tr.hasDocument(id));
    }

    Baloo::FileIndexerConfig cfg;
    QVERIFY(cfg.onlyBasicIndexing());

    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {path});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(tr.hasDocument(id));
        // Verify mimetype
        QVERIFY(tr.documentTerms(id).contains("Mzerosize"));

        QCOMPARE(tr.fetchPhaseOneIds(20).size(), 0);
    }
}

void ModifiedFileIndexerTest::testAddInitialFilled_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QByteArrayList>("mimetypeTerms");

    for (const auto &[name, terms] : fileTestData()) {
        QTest::addRow("%s", qPrintable(name)) << name << terms;
    }
}

void ModifiedFileIndexerTest::testAddInitialFilled()
{
    QFETCH(QString, filename);
    QFETCH(QByteArrayList, mimetypeTerms);

    const auto path{filesDir.filePath(filename)};
    addFile(path, "x");

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(!tr.hasDocument(id));
    }

    Baloo::FileIndexerConfig cfg;
    QVERIFY(cfg.onlyBasicIndexing());

    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {path});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(tr.hasDocument(id));

        // Verify mimetype "application/x-zerosize"
        const auto docTerms{tr.documentTerms(id)};
        QVERIFY(!docTerms.contains("Mzerosize"));

        for (const auto &term : mimetypeTerms) {
            QVERIFY2(docTerms.contains(term), //
                     qPrintable(u"Missing %1 in %2"_s.arg(term).arg(docTerms.join(' '))));
        }

        QCOMPARE(tr.fetchPhaseOneIds(20).size(), 0);
    }
}

void ModifiedFileIndexerTest::testModifyExisting_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QByteArrayList>("mimetypeTerms");

    for (const auto &[name, terms] : fileTestData()) {
        QTest::addRow("%s", qPrintable(name)) << name << terms;
    }
}

void ModifiedFileIndexerTest::testModifyExisting()
{
    QFETCH(QString, filename);
    QFETCH(QByteArrayList, mimetypeTerms);

    const auto path{filesDir.filePath(filename)};
    addFile(path, "");

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(!tr.hasDocument(id));
    }

    Baloo::FileIndexerConfig cfg;
    QVERIFY(cfg.onlyBasicIndexing());

    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {path});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(tr.hasDocument(id));
        // Verify mimetype "application/x-zerosize"
        QVERIFY(tr.documentTerms(id).contains("Mzerosize"));
    }

    addFile(path, "fill");
    // Change the mtime by more than 1 second, otherwise
    // the change is not picked up
    adjustTime(path, QFileDevice::FileModificationTime, 10);
    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {path});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(tr.hasDocument(id));

        const auto docTerms{tr.documentTerms(id)};
        QVERIFY(!docTerms.contains("Mzerosize"));

        for (const auto &term : mimetypeTerms) {
            QVERIFY2(docTerms.contains(term), //
                     qPrintable(u"Missing %1 in %2"_s.arg(term).arg(docTerms.join(' '))));
        }

        QCOMPARE(tr.fetchPhaseOneIds(20).size(), 0);
    }
}

void ModifiedFileIndexerTest::testRename()
{
    const auto path{filesDir.filePath(u"_tempfile"_s)};
    addFile(path, "x");

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(!tr.hasDocument(id));
    }

    Baloo::FileIndexerConfig cfg;
    QVERIFY(cfg.onlyBasicIndexing());

    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {path});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(path));
        QVERIFY(tr.hasDocument(id));
        QVERIFY(tr.documentTerms(id).contains("Moctet"));
    }

    const auto newPath{filesDir.filePath(u"final.txt"_s)};
    QFile::rename(path, newPath);
    // A rename triggers a file ctime change, but ctime is
    // read-only. For this test an mtime change is sufficient
    adjustTime(newPath, QFileDevice::FileModificationTime, 10);

    {
        ModifiedFileIndexer indexer(db.get(), &cfg, {newPath});
        indexer.run();
    }

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        const auto id = filePathToId(QFile::encodeName(newPath));
        QVERIFY(tr.hasDocument(id));

        const auto docTerms{tr.documentTerms(id)};
        QVERIFY(!docTerms.contains("Mzerosize"));

        for (const auto &term : {"Mplain", "Mtext"}) {
            QVERIFY2(docTerms.contains(term), //
                     qPrintable(u"Missing %1 in %2"_s.arg(term).arg(docTerms.join(' '))));
        }

        QCOMPARE(tr.fetchPhaseOneIds(20).size(), 0);
    }
}

QTEST_MAIN(ModifiedFileIndexerTest)

#include "modifiedfileindexertest.moc"
