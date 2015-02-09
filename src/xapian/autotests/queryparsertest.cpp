/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "queryparsertest.h"
#include "../xapianqueryparser.h"
#include "../xapiandatabase.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

void QueryParserTest::testSinglePrefixWord()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The", "F");
    Xapian::Query q("Fthe", 1, 1);
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testSimpleQuery()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The song of Ice and Fire");

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("song", 1, 2);
    queries << Xapian::Query("of", 1, 3);
    queries << Xapian::Query("ice", 1, 4);
    queries << Xapian::Query("and", 1, 5);
    queries << Xapian::Query("fire", 1, 6);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearch()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The \"song of Ice\" Fire");

    QList<Xapian::Query> phraseQueries;
    phraseQueries << Xapian::Query("song", 1, 2);
    phraseQueries << Xapian::Query("of", 1, 3);
    phraseQueries << Xapian::Query("ice", 1, 4);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
    queries << Xapian::Query("fire", 1, 5);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearchOnly()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("/opt/pro");

    QList<Xapian::Query> queries;
    queries << Xapian::Query("opt", 1, 1);
    queries << Xapian::Query("pro", 1, 2);

    Xapian::Query q(Xapian::Query::OP_PHRASE, queries.begin(), queries.end());
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearch_sameLimiter()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The \"song of Ice' and Fire");

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("song", 1, 2);
    queries << Xapian::Query("of", 1, 3);
    queries << Xapian::Query("ice", 1, 4);
    queries << Xapian::Query("and", 1, 5);
    queries << Xapian::Query("fire", 1, 6);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearchEmail()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The song@ice.com Fire");

    QList<Xapian::Query> phraseQueries;
    phraseQueries << Xapian::Query("song", 1, 2);
    phraseQueries << Xapian::Query("ice", 1, 3);
    phraseQueries << Xapian::Query("com", 1, 4);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
    queries << Xapian::Query("fire", 1, 5);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testAccentSearch()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(QString::fromLatin1("sóng"));
    Xapian::Query q("song", 1, 1);

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testUnderscoreSplitting()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery("The_Fire");

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("fire", 1, 2);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

    QCOMPARE(query.serialise(), q.serialise());
}


void QueryParserTest::testWordExpansion()
{
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

    XapianQueryParser parser;
    parser.setDatabase(xap);

    Xapian::Query query = parser.parseQuery("hell");

    QList<Xapian::Query> synQueries;
    synQueries << Xapian::Query("hell", 1, 1);
    synQueries << Xapian::Query("hello", 1, 1);
    synQueries << Xapian::Query("hellog", 1, 1);

    Xapian::Query q(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());

    QCOMPARE(query.serialise(), q.serialise());

    //
    // Try expanding everything
    //
    query = parser.parseQuery("hel hi");

    {
        QList<Xapian::Query> synQueries;
        synQueries << Xapian::Query("hell", 1, 1);
        synQueries << Xapian::Query("hello", 1, 1);
        synQueries << Xapian::Query("hellog", 1, 1);

        Xapian::Query q1(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());

        synQueries.clear();
        synQueries << Xapian::Query("hi", 1, 2);
        synQueries << Xapian::Query("hibrid", 1, 2);

        Xapian::Query q2(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());

        QList<Xapian::Query> queries;
        queries << q1;
        queries << q2;

        Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

        QCOMPARE(query.serialise(), q.serialise());
    }

    {
        Xapian::Query query = parser.parseQuery("rubbish");
        Xapian::Query q = Xapian::Query("rubbish", 1, 1);

        QCOMPARE(query.serialise(), q.serialise());
    }
}



QTEST_MAIN(QueryParserTest)
