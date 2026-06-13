/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "idutils.h"
#include "transaction.h"

#include <QDateTime>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;

class DatabaseCorruptionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAMarkedIndexIsLeftAlone();
    void testAMarkedIndexIsNotOpenedForReading();
    void testPurgingClearsTheMark();
    void testDamagedFileIsReported();

private:
    static QString recordPath(const QTemporaryDir &dir)
    {
        return dir.path() + QStringLiteral("/index-corruption");
    }

    // Stands in for the record the assert handler leaves behind when it finds damage.
    static void markAsDamaged(const QTemporaryDir &dir, const QByteArray &reason = "mdb_page_dirty: Assertion 'rc == 0' failed")
    {
        QFile file(dir.path() + QStringLiteral("/index-corruption"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(reason + '\n');
    }

    static void createIndexWithOneDocument(const QTemporaryDir &dir)
    {
        Database db(dir.path());
        QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);

        Transaction tr(&db, Transaction::ReadWrite);
        Document doc;
        doc.setId(99);
        doc.setParentId(filePathToId(QFile::encodeName(dir.path())));
        doc.setUrl(QFile::encodeName(dir.path() + QStringLiteral("/file")));
        doc.addTerm("power");
        doc.setMTime(1);
        doc.setCTime(2);
        tr.addDocument(doc);
        tr.commit();
    }

    static bool holdsTheDocument(Database &db)
    {
        Transaction tr(&db, Transaction::ReadOnly);
        return tr.hasDocument(99);
    }
};

// A marked index is left alone: baloo does not throw it away by itself, because damage that
// keeps coming back would turn a rebuild into a loop of reindexing and crashing.
void DatabaseCorruptionTest::testAMarkedIndexIsLeftAlone()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    markAsDamaged(dir);

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::InvalidDatabase);
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/index")));
}

// Nothing opens a marked index, so no reader walks back into the damage either.
void DatabaseCorruptionTest::testAMarkedIndexIsNotOpenedForReading()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    markAsDamaged(dir);

    Database db(dir.path());
    QCOMPARE(db.open(Database::ReadOnlyDatabase), Database::OpenResult::InvalidDatabase);
}

// Purging removes the index, which is how the user says to start over. The mark goes with it.
void DatabaseCorruptionTest::testPurgingClearsTheMark()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    markAsDamaged(dir);
    QVERIFY(QFile::remove(dir.path() + QStringLiteral("/index"))); // what balooctl purge does

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/index-corruption")));
    QVERIFY(!holdsTheDocument(db));
}

void DatabaseCorruptionTest::testDamagedFileIsReported()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);

    QFile index(dir.path() + QStringLiteral("/index"));
    QVERIFY(index.open(QIODevice::ReadWrite));
    index.seek(0);
    index.write(QByteArray(4096, '\xAB'));
    index.close();

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::InvalidDatabase);
    // And the damage goes on the record, so the next start rebuilds.
    QVERIFY(QFile::exists(recordPath(dir)));
}

QTEST_GUILESS_MAIN(DatabaseCorruptionTest)

#include "databasecorruptiontest.moc"
