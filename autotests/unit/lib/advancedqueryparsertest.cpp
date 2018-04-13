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

Q_DECLARE_METATYPE(Baloo::Term)
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
    void testNestedParentheses();
    void testNestedParentheses_data();
    void testOptimizedLogic();
    void testOptimizedLogic_data();
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

void AdvancedQueryParserTest::testNestedParentheses()
{
    QFETCH(QString, searchInput);
    QFETCH(QString, failmessage);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    const auto testTerm = parser.parse(searchInput);
    qDebug() << "  result term" << testTerm;
    qDebug() << "expected term" << expectedTerm;
    if (!failmessage.isEmpty()) {
        QEXPECT_FAIL("", qPrintable(failmessage), Continue);
    }

    QCOMPARE(testTerm, expectedTerm);
}

void AdvancedQueryParserTest::testNestedParentheses_data()
{
    QTest::addColumn<QString>("searchInput");
    QTest::addColumn<Term>("expectedTerm");
    QTest::addColumn<QString>("failmessage");

    QTest::newRow("a AND b AND c AND d")
        << QStringLiteral("a AND b AND c AND d")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
        << QString()
        ;
    QTest::newRow("(a AND b) AND (c OR d)")
        << QStringLiteral("(a AND b) AND (c OR d)")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("c"), Term::Contains},
                Term{QString(), QStringLiteral("d"), Term::Contains},
            }}
        }}
        << QString()
        ;
    QTest::newRow("(a AND (b AND (c AND d)))")
        << QStringLiteral("(a AND (b AND (c AND d)))")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
        << QString()
        ;
    // Test 1 for BUG: 392620
    QTest::newRow("a OR ((b AND c) AND d)")
        << QStringLiteral("a OR ((b AND c) AND d)")
        << Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("a"), Term::Contains},
                Term{Term::And, QList<Term>{
                    Term{QString(), QStringLiteral("b"), Term::Contains},
                    Term{QString(), QStringLiteral("c"), Term::Contains},
                    Term{QString(), QStringLiteral("d"), Term::Contains}
                }}
            }}
        << QString()
        ;
    // Test 2 for BUG: 392620
    QTest::newRow("a AND ((b OR c) OR d)")
        << QStringLiteral("a AND ((b OR c) OR d)")
        << Term{Term::And, QList<Term>{
                Term{QString(), QStringLiteral("a"), Term::Contains},
                Term{Term::Or, QList<Term>{
                    Term{QString(), QStringLiteral("b"), Term::Contains},
                    Term{QString(), QStringLiteral("c"), Term::Contains},
                    Term{QString(), QStringLiteral("d"), Term::Contains}
                }}
            }}
        << QString();
        ;
}

void AdvancedQueryParserTest::testOptimizedLogic()
{
    QFETCH(Term, testTerm);
    QFETCH(Term, expectedTerm);
    qDebug() << "  result term" << testTerm;
    qDebug() << "expected term" << expectedTerm;

    QCOMPARE(testTerm, expectedTerm);
}

void AdvancedQueryParserTest::testOptimizedLogic_data()
{
    QTest::addColumn<Term>("testTerm");
    QTest::addColumn<Term>("expectedTerm");

    // a && b && c && d can be combined into one AND term with 4 subterms
    QTest::addRow("a && b && c && d")
        << (Term{QString(), QStringLiteral("a"), Term::Contains}
            && Term{QString(), QStringLiteral("b"), Term::Contains}
            && Term{QString(), QStringLiteral("c"), Term::Contains}
            && Term{QString(), QStringLiteral("d"), Term::Contains})
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
    ;

    // (a AND b) AND (c OR d) can be merged as (a AND b AND (c OR D)
    QTest::addRow("(a && b) && (c || d)")
        << ((Term{QString(), QStringLiteral("a"), Term::Contains}
                && Term{QString(), QStringLiteral("b"), Term::Contains})
            && (Term{QString(), QStringLiteral("c"), Term::Contains}
                || Term{QString(), QStringLiteral("d"), Term::Contains}
            ))
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("c"), Term::Contains},
                Term{QString(), QStringLiteral("d"), Term::Contains}
            }}
        }}
    ;

}

QTEST_MAIN(AdvancedQueryParserTest)

#include "advancedqueryparsertest.moc"
