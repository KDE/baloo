/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014-2015 Vishesh Handa <vhanda@kde.org>
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

#include "queryparser.h"
#include "enginequery.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class QueryParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSinglePrefixWord();
    void testSimpleQuery();
    void testPhraseSearch();
    void testPhraseSearchOnly();
    void testPhraseSearch_sameLimiter();
    void testPhraseSearchEmail();
    void testAccentSearch();
    void testUnderscoreSplitting();

    void testWordExpansion();
};

void QueryParserTest::testSinglePrefixWord()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The", "F");
    EngineQuery q("Fthe", EngineQuery::StartsWith, 1);
    QCOMPARE(query, q);
}

void QueryParserTest::testSimpleQuery()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The song of Ice and Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith, 1);
    queries << EngineQuery("song", EngineQuery::StartsWith, 2);
    queries << EngineQuery("of", EngineQuery::StartsWith, 3);
    queries << EngineQuery("ice", EngineQuery::StartsWith, 4);
    queries << EngineQuery("and", EngineQuery::StartsWith, 5);
    queries << EngineQuery("fire", EngineQuery::StartsWith, 6);

    EngineQuery q(queries, EngineQuery::And);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearch()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The \"song of Ice\" Fire");

    QVector<EngineQuery> phraseQueries;
    phraseQueries << EngineQuery("song", 2);
    phraseQueries << EngineQuery("of", 3);
    phraseQueries << EngineQuery("ice", 4);

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith, 1);
    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
    queries << EngineQuery("fire", EngineQuery::StartsWith, 5);

    EngineQuery q(queries, EngineQuery::And);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearchOnly()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("/opt/pro");

    QVector<EngineQuery> queries;
    queries << EngineQuery("opt", 1);
    queries << EngineQuery("pro", 2);

    EngineQuery q(queries, EngineQuery::Phrase);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearch_sameLimiter()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The \"song of Ice' and Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith, 1);
    queries << EngineQuery("song", EngineQuery::StartsWith, 2);
    queries << EngineQuery("of", EngineQuery::StartsWith, 3);
    queries << EngineQuery("ice", EngineQuery::StartsWith, 4);
    queries << EngineQuery("and", EngineQuery::StartsWith, 5);
    queries << EngineQuery("fire", EngineQuery::StartsWith, 6);

    EngineQuery q(queries, EngineQuery::And);

//    qDebug() << q;
//    qDebug() << query;
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearchEmail()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The song@ice.com Fire");

    QVector<EngineQuery> phraseQueries;
    phraseQueries << EngineQuery("song", 2);
    phraseQueries << EngineQuery("ice", 3);
    phraseQueries << EngineQuery("com", 4);

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith, 1);
    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
    queries << EngineQuery("fire", EngineQuery::StartsWith, 5);

    EngineQuery q(queries, EngineQuery::And);
    // qDebug() << q;
    // qDebug() << query;
    QCOMPARE(query, q);
}

void QueryParserTest::testAccentSearch()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery(QString::fromLatin1("sóng"));
    EngineQuery q("song", EngineQuery::StartsWith, 1);

    QCOMPARE(query, q);
}

void QueryParserTest::testUnderscoreSplitting()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The_Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith, 1);
    queries << EngineQuery("fire", EngineQuery::StartsWith, 2);

    EngineQuery q(queries, EngineQuery::And);

    QCOMPARE(query, q);
}


void QueryParserTest::testWordExpansion()
{
    /*
    QTemporaryDir dir;
    XapianDatabase db(dir.path(), true);

    Xapian::Document doc;
    doc.add_term("hell");
    doc.add_term("hello");
    doc.add_term("hellog");
    doc.add_term("hi");
    doc.add_term("hibrid");

    db.replaceDocument(1, doc);
    Xapian::Database* xap = db.db();

    QueryParser parser;
    parser.setDatabase(xap);

    EngineQuery query = parser.parseQuery("hell");

    QVector<EngineQuery> synQueries;
    synQueries << EngineQuery("hell", 1, 1);
    synQueries << EngineQuery("hello", 1, 1);
    synQueries << EngineQuery("hellog", 1, 1);

    EngineQuery q(EngineQuery::OP_SYNONYM, synQueries.begin(), synQueries.end());

    QCOMPARE(query, q);

    //
    // Try expanding everything
    //
    query = parser.parseQuery("hel hi");

    {
        QVector<EngineQuery> synQueries;
        synQueries << EngineQuery("hell", 1, 1);
        synQueries << EngineQuery("hello", 1, 1);
        synQueries << EngineQuery("hellog", 1, 1);

        EngineQuery q1(EngineQuery::OP_SYNONYM, synQueries.begin(), synQueries.end());

        synQueries.clear();
        synQueries << EngineQuery("hi", 1, 2);
        synQueries << EngineQuery("hibrid", 1, 2);

        EngineQuery q2(EngineQuery::OP_SYNONYM, synQueries.begin(), synQueries.end());

        QVector<EngineQuery> queries;
        queries << q1;
        queries << q2;

        EngineQuery q(EngineQuery::OP_AND, queries.begin(), queries.end());

        QCOMPARE(query, q);
    }

    {
        EngineQuery query = parser.parseQuery("rubbish");
        EngineQuery q = EngineQuery("rubbish", 1, 1);

        QCOMPARE(query, q);
    }
    */
}



QTEST_MAIN(QueryParserTest)

#include "queryparsertest.moc"
