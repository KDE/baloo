/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "database.h"
#include "transaction.h"
#include "document.h"

#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"

#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class Baloo::DatabaseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

static void touchFile(const QByteArray& path) {
    QFile file(QString::fromUtf8(path));
    file.open(QIODevice::WriteOnly);
    file.write("data");
}

void DatabaseTest::test()
{
    QTemporaryDir dir;

    Database db(dir.path());
    QVERIFY(db.open(Database::CreateDatabase));

    Transaction tr(db, Transaction::ReadWrite);

    const QByteArray url(dir.path().toUtf8() + "/file");
    touchFile(url);
    quint64 id = filePathToId(url);

    QCOMPARE(tr.hasDocument(id), false);

    Document doc;
    doc.setId(id);
    doc.setUrl(url);
    doc.addTerm("a");
    doc.addTerm("ab");
    doc.addTerm("abc");
    doc.addTerm("power");
    doc.addXattrTerm("system");
    doc.addFileNameTerm("link");
    doc.setMTime(1);
    doc.setCTime(2);

    tr.addDocument(doc);
    tr.commit();

    Transaction tr2(db, Transaction::ReadOnly);
    QCOMPARE(tr2.hasDocument(id), true);

    PostingDB postingDb(db.m_dbis.postingDbi, tr2.m_txn);
    DocumentUrlDB docUrlDb(db.m_dbis.idTreeDbi, db.m_dbis.idFilenameDbi, tr2.m_txn);

    QCOMPARE(docUrlDb.get(id), url);
    QCOMPARE(postingDb.get("a"), QVector<quint64>() << id);
    QCOMPARE(postingDb.get("ab"), QVector<quint64>() << id);
    QCOMPARE(postingDb.get("abc"), QVector<quint64>() << id);
    QCOMPARE(postingDb.get("power"), QVector<quint64>() << id);
    QCOMPARE(postingDb.get("system"), QVector<quint64>() << id);
    QCOMPARE(postingDb.get("link"), QVector<quint64>() << id);

    /*
    QCOMPARE(db.document(1), doc);

    QVector<int> result = db.exec({"abc"});
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0], 1);

    result = db.exec({"abc", "a"});
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0], 1);

    QCOMPARE(db.documentUrl(1), url);
    QCOMPARE(db.documentId(url), static_cast<quint64>(1));

    db.removeDocument(1);
    QCOMPARE(db.hasDocument(1), false);
    QCOMPARE(db.document(1), Document());

    QCOMPARE(db.documentUrl(1), QByteArray());
    QCOMPARE(db.documentId(url), static_cast<quint64>(0));
    */
}

QTEST_MAIN(DatabaseTest)

#include "databasetest.moc"
