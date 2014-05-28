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

#include "queryparsertest.h"
#include "query.h"
#include "term.h"

#include "../queryparser.h"

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


void QueryParserTest::testSearchString()
{
    QString search_string("correct horse battery staple ");

    QCOMPARE(
        QueryParser::parseQuery(search_string).searchString(),
        search_string
    );
}

void QueryParserTest::testNumbers()
{
    QCOMPARE(
        QueryParser::parseQuery("size > 1024"),
        Query(Term("size", 1024LL, Term::Greater))
    );
}

void QueryParserTest::testDecimal()
{
    QCOMPARE(
        QueryParser::parseQuery("size > 1024.38"),
        Query(Term("size", 1024.38, Term::Greater))
    );
}

void QueryParserTest::testFilesize()
{
    QCOMPARE(
        QueryParser::parseQuery("size > 2K"),
        Query(Term("size", 2048, Term::Greater))
    );
}

void QueryParserTest::testDatetime()
{
    Query expected;
    QDateTime now = QDateTime::currentDateTime();

    // Today
    expected.setDateFilter(now.date().year(), now.date().month(), now.date().day());
    expected.setTerm(Term());

    QCOMPARE(
        QueryParser::parseQuery("today"),
        expected
    );

    // Yesterday
    now = now.addDays(-1);
    expected.setDateFilter(now.date().year(), now.date().month(), now.date().day());

    QCOMPARE(
        QueryParser::parseQuery("yesterday"),
        expected
    );

    // A specific date
    expected.setDateFilter(2011, 1, 2);

    QCOMPARE(
        QueryParser::parseQuery("January 2, 2011"),
        expected
    );
}

void QueryParserTest::testFilename()
{
    QCOMPARE(
        QueryParser::parseQuery("\"*.txt\"", QueryParser::DetectFilenamePattern),
        Query(Term("filename", QRegExp("^.*\\\\.txt$"), Term::Contains))
    );
}

void QueryParserTest::testTypehints()
{
    Query expected;

    expected.setType("Email");

    QCOMPARE(
        QueryParser::parseQuery("emails"),
        expected
    );
}

void QueryParserTest::testReduction()
{
    QCOMPARE(
        QueryParser::parseQuery("size > 2K and size < 3K"),
        Query(Term("size", 2048, Term::Greater) && Term("size", 3072, Term::Less))
    );
    QCOMPARE(
        QueryParser::parseQuery("size > 2K or size < 3K"),
        Query(Term("size", 2048, Term::Greater) || Term("size", 3072, Term::Less))
    );
}

void QueryParserTest::testTags()
{
    QCOMPARE(
        QueryParser::parseQuery("tagged as Important"),
        Query(Term("tags", "Important", Term::Contains))
    );
}

QTEST_KDEMAIN_CORE(QueryParserTest)
