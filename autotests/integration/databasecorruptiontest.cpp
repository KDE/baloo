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
    void testFirstDamageRebuilds();
    void testDamageAfterRebuildStops();
    void testDamageLongAfterRebuildRebuildsAgain();
    void testOtherOpenersWaitForTheRebuild();
    void testDamagedFileIsReported();

private:
    static QString recordPath(const QTemporaryDir &dir)
    {
        return dir.path() + QStringLiteral("/index-corruption");
    }

    // Stands in for the record the assert handler leaves behind when it finds damage.
    static void writeRecord(const QTemporaryDir &dir, int count, const QDateTime &lastRebuild = {})
    {
        QFile file(recordPath(dir));
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write("count=" + QByteArray::number(count) + '\n');
        if (lastRebuild.isValid()) {
            file.write("lastRebuild=" + lastRebuild.toString(Qt::ISODate).toUtf8() + '\n');
        }
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

// Damage seen for the first time buys a rebuild, since it is usually a one-off left behind
// by a hard reboot or the like.
void DatabaseCorruptionTest::testFirstDamageRebuilds()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    writeRecord(dir, 1);

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
    QVERIFY(!holdsTheDocument(db));
}

// Damage that comes back after a rebuild is something that keeps breaking the index, so
// baloo stops rather than reindexing over and over.
void DatabaseCorruptionTest::testDamageAfterRebuildStops()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    writeRecord(dir, 2, QDateTime::currentDateTime());

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::InvalidDatabase);
    // The index is left as it is, for the user to purge by hand once they have reported it.
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/index")));
}

// Damage a long time after the last rebuild is a fresh incident, not a recurring one.
void DatabaseCorruptionTest::testDamageLongAfterRebuildRebuildsAgain()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    writeRecord(dir, 1, QDateTime::currentDateTime().addDays(-400));

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
    QVERIFY(!holdsTheDocument(db));
}

// Only the indexer rebuilds. Everyone else keeps their hands off the index until it has.
void DatabaseCorruptionTest::testOtherOpenersWaitForTheRebuild()
{
    QTemporaryDir dir;
    createIndexWithOneDocument(dir);
    writeRecord(dir, 1);

    Database db(dir.path());
    QCOMPARE(db.open(Database::ReadOnlyDatabase), Database::OpenResult::InvalidDatabase);
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/index")));
}

// Damage in the pages LMDB reads while opening the file comes back as an error code rather
// than through the assert handler, and has to be named for what it is.
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
