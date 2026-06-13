/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "idutils.h"
#include "transaction.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;

class DatabaseCorruptionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMarkerTriggersRebuild();
    void testMarkerRefusedWhenNotCreating();

private:
    static QString markerPath(const QTemporaryDir &dir)
    {
        return dir.path() + QStringLiteral("/index-corrupted");
    }

    static void addOneDocument(Database &db, const QString &dirPath)
    {
        Transaction tr(&db, Transaction::ReadWrite);
        Document doc;
        doc.setId(99);
        doc.setParentId(filePathToId(QFile::encodeName(dirPath)));
        doc.setUrl(QFile::encodeName(dirPath + QStringLiteral("/file")));
        doc.addTerm("power");
        doc.setMTime(1);
        doc.setCTime(2);
        tr.addDocument(doc);
        tr.commit();
    }
};

// A corruption marker (as dropped by the LMDB assert handler) must make the
// owning opener discard the index and rebuild an empty one.
void DatabaseCorruptionTest::testMarkerTriggersRebuild()
{
    QTemporaryDir dir;

    {
        Database db(dir.path());
        QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
        addOneDocument(db, dir.path());
        QVERIFY(QFile::exists(dir.path() + QStringLiteral("/index")));
    }

    {
        QFile marker(markerPath(dir));
        QVERIFY(marker.open(QIODevice::WriteOnly));
        marker.write("simulated corruption");
    }

    Database db(dir.path());
    QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
    // Marker cleared and the index rebuilt empty: the old document is gone.
    QVERIFY(!QFile::exists(markerPath(dir)));
    Transaction tr(&db, Transaction::ReadOnly);
    QCOMPARE(tr.hasDocument(99), false);
}

// A non-owning opener must refuse a flagged index rather than purge it; the
// indexer (CreateDatabase) is responsible for the rebuild.
void DatabaseCorruptionTest::testMarkerRefusedWhenNotCreating()
{
    QTemporaryDir dir;

    {
        Database db(dir.path());
        QCOMPARE(db.open(Database::CreateDatabase), Database::OpenResult::Success);
    }

    {
        QFile marker(markerPath(dir));
        QVERIFY(marker.open(QIODevice::WriteOnly));
        marker.write("simulated corruption");
    }

    Database db(dir.path());
    QCOMPARE(db.open(Database::ReadWriteDatabase), Database::OpenResult::InvalidDatabase);
    // The marker and index are left untouched for the owner to handle.
    QVERIFY(QFile::exists(markerPath(dir)));
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/index")));
}

QTEST_GUILESS_MAIN(DatabaseCorruptionTest)

#include "databasecorruptiontest.moc"
