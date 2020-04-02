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

Q_DECLARE_METATYPE(Baloo::EngineQuery)

using namespace Baloo;

class QueryParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSinglePrefixWord();
    void testSimpleQuery();
    void testPhraseSearch();
    void testPhraseSearchOnly();
    void testUnderscorePhrase();
    void testPhraseSearch_sameLimiter();
    void testPhraseSearchEmail();
    void testAccentSearch();
    void testUnderscoreSplitting();
    void testAutoExpand();
    void testUnicodeLowering();
    void testMixedDelimiters();
    void testMixedDelimiters_data();
};

void QueryParserTest::testSinglePrefixWord()
{
    QueryParser parser;
    parser.setAutoExapandSize(1);

    EngineQuery query = parser.parseQuery("The", "F");
    EngineQuery q("Fthe", EngineQuery::StartsWith);
    QCOMPARE(query, q);
}

void QueryParserTest::testSimpleQuery()
{
    QueryParser parser;
    parser.setAutoExapandSize(1);

    EngineQuery query = parser.parseQuery("The song of Ice and Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith);
    queries << EngineQuery("song", EngineQuery::StartsWith);
    queries << EngineQuery("of", EngineQuery::StartsWith);
    queries << EngineQuery("ice", EngineQuery::StartsWith);
    queries << EngineQuery("and", EngineQuery::StartsWith);
    queries << EngineQuery("fire", EngineQuery::StartsWith);

    EngineQuery q(queries, EngineQuery::And);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearch()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The \"song of Ice\" Fire");

    QVector<EngineQuery> phraseQueries;
    phraseQueries << EngineQuery("song");
    phraseQueries << EngineQuery("of");
    phraseQueries << EngineQuery("ice");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith);
    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
    queries << EngineQuery("fire", EngineQuery::StartsWith);

    EngineQuery q(queries, EngineQuery::And);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearchOnly()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("/opt/pro");

    QVector<EngineQuery> queries;
    queries << EngineQuery("opt");
    queries << EngineQuery("pro");

    EngineQuery q(queries, EngineQuery::Phrase);
    QCOMPARE(query, q);
}

void QueryParserTest::testUnderscorePhrase()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("foo_bar.png");

    QVector<EngineQuery> queries;
    queries << EngineQuery("foo");
    queries << EngineQuery("bar");
    queries << EngineQuery("png");

    EngineQuery q(queries, EngineQuery::Phrase);
    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearch_sameLimiter()
{
    QueryParser parser;
    parser.setAutoExapandSize(1);

    EngineQuery query = parser.parseQuery("The \"song of Ice' and Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith);
    queries << EngineQuery("song", EngineQuery::StartsWith);
    queries << EngineQuery("of", EngineQuery::StartsWith);
    queries << EngineQuery("ice", EngineQuery::StartsWith);
    queries << EngineQuery("and", EngineQuery::StartsWith);
    queries << EngineQuery("fire", EngineQuery::StartsWith);

    EngineQuery q(queries, EngineQuery::And);

    QCOMPARE(query, q);
}

void QueryParserTest::testPhraseSearchEmail()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The song@ice.com Fire");

    QVector<EngineQuery> phraseQueries;
    phraseQueries << EngineQuery("song");
    phraseQueries << EngineQuery("ice");
    phraseQueries << EngineQuery("com");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the", EngineQuery::StartsWith);
    queries << EngineQuery(phraseQueries, EngineQuery::Phrase);
    queries << EngineQuery("fire", EngineQuery::StartsWith);

    EngineQuery q(queries, EngineQuery::And);
    QCOMPARE(query, q);
}

void QueryParserTest::testAccentSearch()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery(QString::fromUtf8("s\xC3\xB3ng")); // sóng
    EngineQuery q("song", EngineQuery::StartsWith);

    QCOMPARE(query, q);
}

void QueryParserTest::testUnderscoreSplitting()
{
    QueryParser parser;

    EngineQuery query = parser.parseQuery("The_Fire");

    QVector<EngineQuery> queries;
    queries << EngineQuery("the");
    queries << EngineQuery("fire");

    EngineQuery q(queries, EngineQuery::Phrase);

    QCOMPARE(query, q);

    query = parser.parseQuery("_Fire");
    q = EngineQuery("fire", EngineQuery::StartsWith);

    QCOMPARE(query, q);
}

void QueryParserTest::testAutoExpand()
{
    QueryParser parser;
    parser.setAutoExapandSize(0);

    {
        EngineQuery query = parser.parseQuery("the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal);
        queries << EngineQuery("fire", EngineQuery::Equal);

        EngineQuery q(queries, EngineQuery::And);

        QCOMPARE(query, q);
    }

    {
        EngineQuery query = parser.parseQuery("'the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal);
        queries << EngineQuery("fire", EngineQuery::Equal);

        EngineQuery q(queries, EngineQuery::And);

        QCOMPARE(query, q);
    }

    parser.setAutoExapandSize(4);
    {
        EngineQuery query = parser.parseQuery("the fire");

        QVector<EngineQuery> queries;
        queries << EngineQuery("the", EngineQuery::Equal);
        queries << EngineQuery("fire", EngineQuery::StartsWith);

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
    EngineQuery expected = EngineQuery("hedge", EngineQuery::StartsWith);
    QCOMPARE(query, expected);
}

void QueryParserTest::testMixedDelimiters()
{
    QFETCH(QString, input);
    QFETCH(EngineQuery, expectedQuery);
    QFETCH(QString, failureReason);

    QueryParser parser;
    parser.setAutoExapandSize(0);
    EngineQuery query = parser.parseQuery(input);
    if (!failureReason.isEmpty()) {
        QEXPECT_FAIL("", qPrintable(failureReason), Continue);
    }
    QCOMPARE(query, expectedQuery);
}

void QueryParserTest::testMixedDelimiters_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<EngineQuery>("expectedQuery");
    QTest::addColumn<QString>("failureReason");

    auto addRow = [](const QString& input, const EngineQuery& query,
                     const QString& failureReason)
        { QTest::addRow("%s", qPrintable(input)) << input << query << failureReason; };

    addRow("Term", {"term"}, "");
    addRow("No phrase", { {{"no"}, {"phrase"}}, EngineQuery::And}, "");
    addRow("Underscore_phrase", { {{"underscore"}, {"phrase"}}, EngineQuery::Phrase}, "");
    addRow("underscore_dot.phrase", { {{"underscore"}, {"dot"}, {"phrase"}}, EngineQuery::Phrase}, "");
    addRow("\'Quoted phrase\'", { {{"quoted"}, {"phrase"}}, EngineQuery::Phrase}, "End quote as last char");
    addRow("\'Quoted phrase\' anded tail", {{
	    {{{"quoted"}, {"phrase"}}, EngineQuery::Phrase},
	    {"anded"}, {"tail"},
        }, EngineQuery::And}, "");
    addRow("\'Long quoted phrase\'", { {{"long"}, {"quoted"}, {"phrase"}}, EngineQuery::Phrase}, "End quote as last char");
    addRow("Anded dot.phrase", { {
            {"anded"},
            {{{"dot"}, {"phrase"}}, EngineQuery::Phrase},
        }, EngineQuery::And}, "");
    addRow("Under_score dot.phrase", {{
            {{{"under"}, {"score"}}, EngineQuery::Phrase},
            {{{"dot"}, {"phrase"}}, EngineQuery::Phrase},
        }, EngineQuery::And}, "");
    addRow("\'One quoted\' Other.withDot", {{
            {{{"one"}, {"quoted"}}, EngineQuery::Phrase},
            {{{"other"}, {"withdot"}}, EngineQuery::Phrase},
        }, EngineQuery::And}, "");
    addRow("\'One quoted with.dot\'", {{
            {"one"}, {"quoted"}, {"with"}, {"dot"}
        }, EngineQuery::Phrase}, "End quote as last char");
    addRow("\'Quoted_underscore and.dot\'", {{
            {"quoted"}, {"underscore"}, {"and"}, {"dot"}
        }, EngineQuery::Phrase}, "End quote as last char");
    addRow("Underscore_andTrailingDot_.", {{
            {"underscore"}, {"andtrailingdot"}
        }, EngineQuery::Phrase}, "");
    addRow("\'TrailingUnderscore_ andDot.\'", {{
            {"trailingunderscore"}, {"anddot"}
        }, EngineQuery::Phrase}, "End quote as last char");
    addRow("NoPhrase Under_score \'Quoted Phrase\'", {{
            {"nophrase"},
            {{{"under"}, {"score"}}, EngineQuery::Phrase},
            {{{"quoted"}, {"phrase"}}, EngineQuery::Phrase},
        }, EngineQuery::And}, "End quote as last char");
    addRow("NoPhrase \'Quoted Phrase\' Under_score", {{
            {"nophrase"},
            {{{"quoted"}, {"phrase"}}, EngineQuery::Phrase},
            {{{"under"}, {"score"}}, EngineQuery::Phrase},
        }, EngineQuery::And}, "");
    addRow("\'DegeneratedQuotedPhrase\' Anded text", {
            {{"degeneratedquotedphrase"}, {"anded"}, {"text"}}, EngineQuery::And
        }, "Single term in quotes is no phrase");
}

QTEST_MAIN(QueryParserTest)

#include "queryparsertest.moc"
