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
#include "../queryparser.h"

#include <QTest>
#include <QDebug>

using namespace Baloo;

void QueryParserTest::testSimpleQuery()
{
    QueryParser parser;

    Xapian::Query query = parser.parseQuery("The song of Ice and Fire");

    QList<std::string> terms;
    terms << "the" << "song" << "of" << "ice" << "and" << "fire";

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

QTEST_MAIN(QueryParserTest)
