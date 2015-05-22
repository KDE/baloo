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
#include "documentdatadb.h"
#include "positiondb.h"
#include "documenttimedb.h"

#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class DBState {
public:
    QMap<QByteArray, PostingList> postingDb;
    QMap<QByteArray, QVector<PositionInfo>> positionDb;

    QMap<quint64, QVector<QByteArray>> docTermsDb;
    QMap<quint64, QVector<QByteArray>> docFileNameTermsDb;
    QMap<quint64, QVector<QByteArray>> docXAttrTermsDb;

    QMap<quint64, DocumentTimeDB::TimeInfo> docTimeDb;
    QMap<quint32, quint64> mtimeDb;

    QMap<quint64, QByteArray> docDataDb;
    QMap<quint64, QByteArray> docUrlDb;
    QVector<quint64> contentIndexingDb;

    bool operator== (const DBState& st) const {
        return postingDb == st.postingDb && positionDb == st.positionDb && docTermsDb == st.docTermsDb
               && docFileNameTermsDb == st.docFileNameTermsDb && docXAttrTermsDb == st.docXAttrTermsDb
               && docTimeDb == st.docTimeDb && mtimeDb == st.mtimeDb && docDataDb == st.docDataDb
               && docUrlDb == st.docUrlDb && contentIndexingDb == st.contentIndexingDb;
    }
private:
};

class Baloo::DatabaseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();


private:
    DBState toState(Transaction* txn);
};

DBState DatabaseTest::toState(Transaction* tr)
{
    auto dbis = tr->m_dbis;
    MDB_txn* txn = tr->m_txn;

    PostingDB postingDB(dbis.postingDbi, txn);
    PositionDB positionDB(dbis.positionDBi, txn);
    DocumentDB documentTermsDB(dbis.docTermsDbi, txn);
    DocumentDB documentXattrTermsDB(dbis.docXattrTermsDbi, txn);
    DocumentDB documentFileNameTermsDB(dbis.docFilenameTermsDbi, txn);
    DocumentTimeDB docTimeDB(dbis.docTimeDbi, txn);
    DocumentDataDB docDataDB(dbis.docDataDbi, txn);
    DocumentIdDB contentIndexingDB(dbis.contentIndexingDbi, txn);
    MTimeDB mtimeDB(dbis.mtimeDbi, txn);
    DocumentUrlDB docUrlDB(dbis.idTreeDbi, dbis.idFilenameDbi, txn);

    DBState state;
    state.postingDb = postingDB.toTestMap();
    state.positionDb = positionDB.toTestMap();
    state.docTermsDb = documentTermsDB.toTestMap();
    state.docXAttrTermsDb = documentXattrTermsDB.toTestMap();
    state.docFileNameTermsDb = documentFileNameTermsDB.toTestMap();
    state.docTimeDb = docTimeDB.toTestMap();
    state.docDataDb = docDataDB.toTestMap();
    state.mtimeDb = mtimeDB.toTestMap();
    state.contentIndexingDb = contentIndexingDB.toTestVector();

    // FIXME: What about DocumentUrlDB?

    return state;
}

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

    DBState actualState = toState(&tr2);
    QCOMPARE(actualState, state);
    // FIXME: Check url?
}

QTEST_MAIN(DatabaseTest)

#include "databasetest.moc"
