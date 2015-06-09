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
    void testAddDocumentTwoDocuments();
    void testAddAndRemoveOneDocument();

    void testRemoveRecursively();
    void testDocumentId();
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


static Document createDocument(const QByteArray& url, quint32 mtime, quint32 ctime, const QVector<QByteArray>& terms,
                               const QVector<QByteArray>& fileNameTerms, const QVector<QByteArray>& xattrTerms)
{
    Document doc;
    doc.setId(filePathToId(url));
    doc.setUrl(url);

    for (const QByteArray& term: terms) {
        doc.addTerm(term);
    }
    for (const QByteArray& term: fileNameTerms) {
        doc.addFileNameTerm(term);
    }
    for (const QByteArray& term: xattrTerms) {
        doc.addXattrTerm(term);
    }
    doc.setMTime(mtime);
    doc.setCTime(ctime);

    return doc;
}

void WriteTransactionTest::testAddDocumentTwoDocuments()
{
    const QByteArray url1(dir->path().toUtf8() + "/file1");
    const QByteArray url2(dir->path().toUtf8() + "/file2");
    touchFile(url1);
    touchFile(url2);

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {});
    Document doc2 = createDocument(url2, 6, 2, {"a", "abcd", "dab"}, {"file2"}, {});

    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.addDocument(doc2);
        tr.commit();
    }

    Transaction tr(db, Transaction::ReadOnly);

    quint64 id1 = doc1.id();
    quint64 id2 = doc2.id();

    DBState state;
    state.postingDb = {{"a", {id1, id2}}, {"abc", {id1}}, {"abcd", {id2}}, {"dab", {id1, id2}}, {"file1", {id1}}, {"file2", {id2}}};
    state.positionDb = {};
    state.docTermsDb = {{id1, {"a", "abc", "dab"}}, {id2, {"a", "abcd", "dab"}}};
    state.docFileNameTermsDb = {{id1, {"file1"}}, {id2, {"file2"}}};
    state.docXAttrTermsDb = {};
    state.docTimeDb = {{id1, DocumentTimeDB::TimeInfo(5, 1)}, {id2, DocumentTimeDB::TimeInfo(6, 2)}};
    state.mtimeDb = {{5, id1}, {6, id2}};

    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, state));
}

void WriteTransactionTest::testAddAndRemoveOneDocument()
{
    const QByteArray url1(dir->path().toUtf8() + "/file1");
    touchFile(url1);

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {});

    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }
    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.removeDocument(doc1.id());
        tr.commit();
    }

    Transaction tr(db, Transaction::ReadOnly);
    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, DBState()));
}

void WriteTransactionTest::testRemoveRecursively()
{
    QByteArray path = dir->path().toUtf8();

    const QByteArray url1(path + "/file1");
    touchFile(url1);
    const QByteArray dirPath(path + "/dir");
    QDir().mkpath(QString::fromUtf8(dirPath));
    const QByteArray url2(dirPath + "/file1");
    touchFile(url2);

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {});
    Document doc2 = createDocument(url2, 5, 1, {"a", "abc", "dab"}, {"file1"}, {});
    Document doc3 = createDocument(dirPath, 5, 1, {"a"}, {"dir"}, {});

    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.addDocument(doc2);
        tr.addDocument(doc3);
        tr.commit();
    }
    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.removeRecursively(filePathToId(dir->path().toUtf8()));
        tr.commit();
    }

    Transaction tr(db, Transaction::ReadOnly);
    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, DBState()));
}

void WriteTransactionTest::testDocumentId()
{
    const QByteArray url1(dir->path().toUtf8() + "/file1");
    touchFile(url1);

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {});

    {
        Transaction tr(db, Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }

    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.documentId(url1), doc1.id());
}


QTEST_MAIN(WriteTransactionTest)

#include "writetransactiontest.moc"
