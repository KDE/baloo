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
#include "document.h"
#include "termgenerator.h"
#include "enginequery.h"

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
        db->open();

        db->transaction(Database::ReadWrite);
        insertDocuments();
        db->commit();
        db->transaction(Database::ReadOnly);
    }

    void cleanup() {
        delete db;
        delete dir;
    }

    void testTermEqual();
    void testTermStartsWith();
    void testTermAnd();
    void testTermOr();

private:
    QTemporaryDir* dir;
    Database* db;

    void insertDocuments();
    void addDocument(const QString& text, uint id, const QByteArray& url)
    {
        Document doc;

        TermGenerator tg(&doc);
        tg.indexText(text);
        doc.setId(id);
        doc.setUrl(url);

        db->addDocument(doc);
    }
};


void QueryTest::insertDocuments()
{
    addDocument("The quick brown foxed jumped over the crazy dog", 100, "/h/v/fire");
    addDocument("The night is dark and full of terror", 110, "/h/v/water");
    addDocument("Don't feel sorry for yourself. Only assholes do that", 120, "/h/v/air");
    addDocument("Only the dead stay 17 forever", 130, "/h/v/earth");
}

void QueryTest::testTermEqual()
{
    EngineQuery q("the");

    QVector<uint> result = {100, 110, 130};
    QCOMPARE(db->exec(q), result);
}

void QueryTest::testTermStartsWith()
{
    EngineQuery q("for", EngineQuery::StartsWith);

    QVector<uint> result = {120, 130};
    QCOMPARE(db->exec(q), result);
}

void QueryTest::testTermAnd()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("for");
    queries << EngineQuery("sorry");

    EngineQuery q(queries, EngineQuery::And);

    QVector<uint> result = {120};
    QCOMPARE(db->exec(q), result);
}

void QueryTest::testTermOr()
{
    QVector<EngineQuery> queries;
    queries << EngineQuery("over");
    queries << EngineQuery("terror");

    EngineQuery q(queries, EngineQuery::Or);

    QVector<uint> result = {100, 110};
    QCOMPARE(db->exec(q), result);
}


QTEST_MAIN(QueryTest)

#include "querytest.moc"
