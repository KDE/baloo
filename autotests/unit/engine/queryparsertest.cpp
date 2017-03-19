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
    void testAutoExpand();
    void testUnicodeLowering();
};

void QueryParserTest::testSinglePrefixWord()
{
    QueryParser parser;
    parser.setAutoExapandSize(1);

    EngineQuery query = parser.parseQuery("The", "F");
    EngineQuery q("Fthe", EngineQuery::StartsWith, 1);
    QCOMPARE(query, q);
}

void QueryParserTest::testSimpleQuery()
{
    QueryParser parser;
    parser.setAutoExapandSize(1);

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
    parser.setAutoExapandSize(1);

    EngineQuery query = parser.parseQuery("The \"song of Ice' and Fire");

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

    query = parser.parseQuery("_Fire");
    q = EngineQuery("fire", EngineQuery::StartsWith, 1);

    QCOMPARE(query, q);
}

void QueryParserTest::testAutoExpand()
{
    QueryParser parser;
    parser.setAutoExapandSize(0);

    {
        EngineQuery query = parser.parseQuery("the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal, 1);
        queries << EngineQuery("fire", EngineQuery::Equal, 2);

        EngineQuery q(queries, EngineQuery::And);

        QCOMPARE(query, q);
    }

    {
        EngineQuery query = parser.parseQuery("'the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal, 1);
        queries << EngineQuery("fire", EngineQuery::Equal, 2);

        EngineQuery q(queries, EngineQuery::And);

        QCOMPARE(query, q);
    }

    parser.setAutoExapandSize(4);
    {
        EngineQuery query = parser.parseQuery("the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal, 1);
        queries << EngineQuery("fire", EngineQuery::StartsWith, 2);

        EngineQuery q(queries, EngineQuery::And);

        QCOMPARE(query, q);
    }
}

void QueryParserTest::testUnicodeLowering()
{
    // This string is unicode mathematical italic "Hedge"
    QString str = QString::fromUtf8("\xF0\x9D\x90\xBB\xF0\x9D\x91\x92\xF0\x9D\x91\x91\xF0\x9D\x91\x94\xF0\x9D\x91\x92");

    QueryParser parser;
    EngineQuery query = parser.parseQuery(str);
    EngineQuery expected = EngineQuery("hedge", EngineQuery::StartsWith, 1);
    QCOMPARE(query, expected);
}

QTEST_MAIN(QueryParserTest)

#include "queryparsertest.moc"
