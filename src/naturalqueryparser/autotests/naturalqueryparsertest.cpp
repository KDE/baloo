/*
 * This file is part of the Baloo Query Parser
 * Copyright (C) 2014  Denis Steckelmacher <steckdenis@yahoo.fr>
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

#include "naturalqueryparsertest.h"
#include "query.h"
#include "term.h"

#include "../naturalqueryparser.h"

#include <qtest_kde.h>

#include <QtCore/QDateTime>

using namespace Baloo;

namespace QTest {
    template<>
    char *toString(const Query &query)
    {
        Query *q = const_cast<Query *>(&query);    // query.toJSON does not modify query but is not marked const

        return qstrdup(q->toJSON().constData());
    }
}


void NaturalQueryParserTest::testSearchString()
{
    QString search_string(QLatin1String("correct horse battery staple "));

    QCOMPARE(
        NaturalQueryParser::parseQuery(search_string).searchString(),
        search_string
    );
}

void NaturalQueryParserTest::testNumbers()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("size > 1024")),
        Query(Term(QLatin1String("size"), 1024LL, Term::Greater))
    );
}

void NaturalQueryParserTest::testDecimal()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("size > 1024.38")),
        Query(Term(QLatin1String("size"), 1024.38, Term::Greater))
    );
}

void NaturalQueryParserTest::testFilesize()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("size > 2K")),
        Query(Term(QLatin1String("size"), 2048, Term::Greater))
    );
}

void NaturalQueryParserTest::testDatetime()
{
    Query expected;
    QDateTime now = QDateTime::currentDateTime();

    // Today
    expected.setDateFilter(now.date().year(), now.date().month(), now.date().day());
    expected.setTerm(Term());

    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("today")),
        expected
    );

    // Yesterday
    now = now.addDays(-1);
    expected.setDateFilter(now.date().year(), now.date().month(), now.date().day());

    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("yesterday")),
        expected
    );

    // A specific date
    expected.setDateFilter(2011, 1, 2);

    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("January 2, 2011")),
        expected
    );
}

void NaturalQueryParserTest::testFilename()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("\"*.txt\""), NaturalQueryParser::DetectFilenamePattern),
        Query(Term(QLatin1String("filename"), QRegExp(QLatin1String("^.*\\\\.txt$")), Term::Contains))
    );
}

void NaturalQueryParserTest::testTypehints()
{
    Query expected;

    expected.setType(QLatin1String("Email"));

    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("emails")),
        expected
    );
}

void NaturalQueryParserTest::testReduction()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("size > 2K and size < 3K")),
        Query(Term(QLatin1String("size"), 2048, Term::Greater) && Term(QLatin1String("size"), 3072, Term::Less))
    );
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("size > 2K or size < 3K")),
        Query(Term(QLatin1String("size"), 2048, Term::Greater) || Term(QLatin1String("size"), 3072, Term::Less))
    );
}

void NaturalQueryParserTest::testTags()
{
    QCOMPARE(
        NaturalQueryParser::parseQuery(QLatin1String("tagged as Important")),
        Query(Term(QLatin1String("tags"), QLatin1String("Important"), Term::Contains))
    );
}

QTEST_KDEMAIN_CORE(NaturalQueryParserTest)
