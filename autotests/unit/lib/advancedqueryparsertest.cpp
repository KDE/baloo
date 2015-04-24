/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
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

#include "advancedqueryparser.h"

#include <QTest>

using namespace Baloo;

class AdvancedQueryParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSimpleProperty();
    void testSimpleString();
    void testStringAndProperty();
    void testLogicalOps();
    void testNesting();
    void testDifferentTypes();
};

void AdvancedQueryParserTest::testSimpleProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse("artist:Coldplay");
    Term expectedTerm("artist", "Coldplay");

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testSimpleString()
{
    AdvancedQueryParser parser;
    Term term = parser.parse("Coldplay");
    Term expectedTerm("", "Coldplay");

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testStringAndProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse("stars artist:Coldplay fire");
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term("", "stars"));
    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("", "fire"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testLogicalOps()
{
    // AND
    AdvancedQueryParser parser;
    Term term = parser.parse("artist:Coldplay AND type:song");
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("type", "song"));

    QCOMPARE(term, expectedTerm);

    // OR
    term = parser.parse("artist:Coldplay OR type:song");
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("type", "song"));

    QCOMPARE(term, expectedTerm);

    // AND then OR
    term = parser.parse("artist:Coldplay AND type:song OR stars");
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term("artist", "Coldplay") && Term("type", "song"));
    expectedTerm.addSubTerm(Term("", "stars"));

    QCOMPARE(term, expectedTerm);

    // OR then AND
    term = parser.parse("artist:Coldplay OR type:song AND stars");
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term("artist", "Coldplay") || Term("type", "song"));
    expectedTerm.addSubTerm(Term("", "stars"));

    QCOMPARE(term, expectedTerm);

    // Multiple ANDs
    term = parser.parse("artist:Coldplay AND type:song AND stars");
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("type", "song"));
    expectedTerm.addSubTerm(Term("", "stars"));

    QCOMPARE(term, expectedTerm);

    // Multiple ORs
    term = parser.parse("artist:Coldplay OR type:song OR stars");
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("type", "song"));
    expectedTerm.addSubTerm(Term("", "stars"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testNesting()
{
    AdvancedQueryParser parser;
    Term term = parser.parse("artist:Coldplay AND (type:song OR stars) fire");
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term("artist", "Coldplay"));
    expectedTerm.addSubTerm(Term("type", "song") || Term("", "stars"));
    expectedTerm.addSubTerm(Term("", "fire"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testDifferentTypes()
{
    // Integers
    AdvancedQueryParser parser;
    Term term = parser.parse("width:500");

    Term expectedTerm("width", 500);
    QCOMPARE(term, expectedTerm);

    term = parser.parse("width<500");

    expectedTerm = Term("width", 500, Term::Less);
    QCOMPARE(term, expectedTerm);

    // Date
    term = parser.parse("modified:2014-12-02");

    expectedTerm = Term("modified", QDate(2014, 12, 02));
    QCOMPARE(term, expectedTerm);
}

QTEST_MAIN(AdvancedQueryParserTest);

#include "advancedqueryparsertest.moc"
