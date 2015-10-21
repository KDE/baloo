/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

    const QByteArray url(dir->path().toUtf8() + "/file");
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
