/*
 * This file is part of the KDE Baloo project.
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

#include "writetransaction.h"
#include "dbstate.h"
#include "database.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class WriteTransactionTest : public QObject
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

    void testAddDocument();

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

void WriteTransactionTest::testAddDocument()
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
    doc.addXattrTerm("system");
    doc.setMTime(1);
    doc.setCTime(2);

    tr.addDocument(doc);
    tr.commit();

    Transaction tr2(db, Transaction::ReadOnly);

    DBState state;
    state.postingDb = {{"a", {id}}, {"ab", {id}}, {"abc", {id}}, {"power", {id}}, {"system", {id}}, {"link", {id}}};
    state.positionDb = {};
    state.docTermsDb = {{id, {"a", "ab", "abc", "power"} }};
    state.docFileNameTermsDb = {{id, {"link"} }};
    state.docXAttrTermsDb = {{id, {"system"} }};
    state.docTimeDb = {{id, DocumentTimeDB::TimeInfo(1, 2)}};
    state.mtimeDb = {{1, id}};

    DBState actualState = DBState::fromTransaction(&tr2);
    QCOMPARE(actualState, state);
}



QTEST_MAIN(WriteTransactionTest)

#include "writetransactiontest.moc"
