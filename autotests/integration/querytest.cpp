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
#include "termgenerator.h"
#include "enginequery.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class QueryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        dir = new QTemporaryDir();
        db = new Database(dir->path());
        db->open(Database::CreateDatabase);

        Transaction tr(db, Transaction::ReadWrite);
        insertDocuments(&tr);
        tr.commit();
    }

    void cleanup() {
        delete db;
        delete dir;
    }

    void testTermEqual();
    void testTermStartsWith();
    void testTermAnd();
    void testTermOr();
    void testTermPhrase();

private:
    QTemporaryDir* dir;
    Database* db;

    void insertDocuments(Transaction* tr);
    void addDocument(Transaction* tr,const QString& text, quint64 id, const QString& url)
    {
        Document doc;
        doc.setUrl(QFile::encodeName(url));

        QString fileName = url.mid(url.lastIndexOf('/') + 1);

        TermGenerator tg(&doc);
        tg.indexText(text);
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setMTime(1);
        doc.setCTime(2);

        tr->addDocument(doc);
    }

    quint64 m_id1;
    quint64 m_id2;
    quint64 m_id3;
    quint64 m_id4;
};

static quint64 touchFile(const QString& path) {
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("data");
    file.close();

    return filePathToId(QFile::encodeName(path));
}

void QueryTest::insertDocuments(Transaction* tr)
{
    m_id1 = touchFile(dir->path() + "/file1");
    m_id2 = touchFile(dir->path() + "/file2");
    m_id3 = touchFile(dir->path() + "/file3");
    m_id4 = touchFile(dir->path() + "/file4");

    addDocument(tr, QStringLiteral("The quick brown foxed jumped over the crazy dog"), m_id1, dir->path() + "/file1");
    addDocument(tr, QStringLiteral("The night is dark and full of terror"), m_id2, dir->path() + "/file2");
    addDocument(tr, QStringLiteral("Don't feel sorry for yourself. Only assholes do that"), m_id3, dir->path() + "/file3");
    addDocument(tr, QStringLiteral("Only the dead stay 17 forever. crazy"), m_id4, dir->path() + "/file4");
}

void QueryTest::testTermEqual()
{
    EngineQuery q("the");

    QVector<quint64> result = {m_id1, m_id2, m_id4};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermStartsWith()
{
    EngineQuery q("for", EngineQuery::StartsWith);

    QVector<quint64> result = {m_id3, m_id4};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermAnd()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("for");
    queries << EngineQuery("sorry");

    EngineQuery q(queries, EngineQuery::And);

    QVector<quint64> result = {m_id3};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermOr()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("over");
    queries << EngineQuery("terror");

    EngineQuery q(queries, EngineQuery::Or);

    QVector<quint64> result = {m_id1, m_id2};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}

void QueryTest::testTermPhrase()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("the");
    queries << EngineQuery("crazy");

    EngineQuery q(queries, EngineQuery::Phrase);

    QVector<quint64> result = {m_id1};
    Transaction tr(db, Transaction::ReadOnly);
    QCOMPARE(tr.exec(q), result);
}


QTEST_MAIN(QueryTest)

#include "querytest.moc"
