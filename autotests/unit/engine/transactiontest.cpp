/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "transaction.h"
#include "database.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class TransactionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        dir = new QTemporaryDir();
        db = new Database(dir->path());
        db->open(Database::CreateDatabase);
    }

    void cleanup() {
        delete db;
        delete dir;
    }

    void testTimeInfo();
private:
    QTemporaryDir* dir;
    Database* db;
};

static quint64 touchFile(const QString& path) {
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("data");
    file.close();

    return filePathToId(QFile::encodeName(path));
}

void TransactionTest::testTimeInfo()
{
    Transaction tr(db, Transaction::ReadWrite);

    const QString url(dir->path() + QStringLiteral("/file"));
    touchFile(url);
    quint64 id = filePathToId(QFile::encodeName(url));

    QCOMPARE(tr.hasDocument(id), false);

    Document doc;
    doc.setId(id);
    doc.setUrl(QFile::encodeName(url));
    doc.addTerm("a");
    doc.addTerm("ab");
    doc.addTerm("abc");
    doc.addTerm("power");
    doc.addFileNameTerm("link");
    doc.setMTime(1);
    doc.setCTime(2);

    tr.addDocument(doc);
    tr.commit();

    Transaction tr2(db, Transaction::ReadOnly);

    DocumentTimeDB::TimeInfo timeInfo(1, 2);
    QCOMPARE(tr2.documentTimeInfo(id), timeInfo);
}


QTEST_MAIN(TransactionTest)

#include "transactiontest.moc"
