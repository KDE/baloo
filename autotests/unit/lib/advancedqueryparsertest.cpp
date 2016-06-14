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
    void testDateTime();
    void testOperators();
    void testBinaryOperatorMissingFirstArg();
};

void AdvancedQueryParserTest::testSimpleProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay"));
    Term expectedTerm(QStringLiteral("artist"), "Coldplay");

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testSimpleString()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("Coldplay"));
    Term expectedTerm(QLatin1String(""), "Coldplay");

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testStringAndProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("stars artist:Coldplay fire"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QLatin1String(""), "stars"));
    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "fire"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testLogicalOps()
{
    // AND
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay AND type:song"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), "song"));

    QCOMPARE(term, expectedTerm);

    // OR
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), "song"));

    QCOMPARE(term, expectedTerm);

    // AND then OR
    term = parser.parse(QStringLiteral("artist:Coldplay AND type:song OR stars"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay") && Term(QStringLiteral("type"), "song"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "stars"));

    QCOMPARE(term, expectedTerm);

    // OR then AND
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song AND stars"));
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay") || Term(QStringLiteral("type"), "song"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "stars"));

    QCOMPARE(term, expectedTerm);

    // Multiple ANDs
    term = parser.parse(QStringLiteral("artist:Coldplay AND type:song AND stars"));
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), "song"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "stars"));

    QCOMPARE(term, expectedTerm);

    // Multiple ORs
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song OR stars"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), "song"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "stars"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testNesting()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay AND (type:song OR stars) fire"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), "Coldplay"));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), "song") || Term(QLatin1String(""), "stars"));
    expectedTerm.addSubTerm(Term(QLatin1String(""), "fire"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testDateTime()
{
    // Integers
    AdvancedQueryParser parser;
    Term term;
    Term expectedTerm;

    term = parser.parse(QStringLiteral("modified:2014-12-02"));
    expectedTerm = Term(QStringLiteral("modified"), QDate(2014, 12, 02));
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("modified:\"2014-12-02T23:22:1\""));
    expectedTerm = Term(QStringLiteral("modified"), QDateTime(QDate(2014, 12, 02), QTime(23, 22, 1)));
    QEXPECT_FAIL("", "AQP cannot handle datetime", Abort);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testOperators()
{
    AdvancedQueryParser parser;
    Term term;
    Term expectedTerm;

    term = parser.parse(QStringLiteral("width:500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::Equal);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width=500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::Equal);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width<500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::Less);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width<=500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::LessEqual);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width>500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::Greater);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width>=500"));
    expectedTerm = Term(QStringLiteral("width"), 500, Term::GreaterEqual);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testBinaryOperatorMissingFirstArg()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("=:2"));
    Term expectedTerm;
    QCOMPARE(term, expectedTerm);
}


QTEST_MAIN(AdvancedQueryParserTest)

#include "advancedqueryparsertest.moc"
