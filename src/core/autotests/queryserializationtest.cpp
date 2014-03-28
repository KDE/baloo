/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "queryserializationtest.h"
#include "query.h"
#include <term.h>
#include "term.h"

#include <qtest_kde.h>

#include <KDebug>

using namespace Baloo;

// Test a simple query with no terms
void QuerySerializationTest::testBasic()
{
    Query query;
    query.setLimit(5);
    query.setOffset(1);
    query.setSearchString("Bookie");
    query.addType("File/Audio");

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q.limit(), static_cast<uint>(5));
    QCOMPARE(q.offset(), static_cast<uint>(1));
    QCOMPARE(q.searchString(), QLatin1String("Bookie"));
    QCOMPARE(q.types().size(), 2);
    QVERIFY(q.types().contains("File"));
    QVERIFY(q.types().contains("Audio"));

    QCOMPARE(q, query);
}

void QuerySerializationTest::testTerm()
{
    Term term("property", "value");

    Query query;
    query.setTerm(term);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    Term t = q.term();
    QCOMPARE(t.property(), QLatin1String("property"));
    QCOMPARE(t.value(), QVariant("value"));
    QCOMPARE(q.term(), term);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testAndTerm()
{
    Term term1("prop1", 1);
    Term term2("prop2", 2);

    Term term(Term::And);
    term.addSubTerm(term1);
    term.addSubTerm(term2);

    Query query;
    query.setTerm(term);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    Term t = q.term();
    QCOMPARE(t.subTerms().size(), 2);
    QCOMPARE(t.subTerms().at(0), term1);
    QCOMPARE(t.subTerms().at(1), term2);

    QCOMPARE(t, term);
    QCOMPARE(q, query);

}

void QuerySerializationTest::testDateTerm()
{
    Term term("prop", QDate::currentDate());

    Query query;
    query.setTerm(term);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    Term t = q.term();
    QCOMPARE(t.value(), term.value());
    QCOMPARE(t.value().typeName(), term.value().typeName());
    QCOMPARE(t.property(), term.property());
}

void QuerySerializationTest::testDateTimeTerm()
{
    // This is hack being done so that the milliseconds are ignored
    // the internal QJson serializer throws away the msecs
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime(dt.time().hour(), dt.time().minute(), dt.time().second()));

    Term term("prop", dt);

    Query query;
    query.setTerm(term);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    Term t = q.term();
    QCOMPARE(t.value().typeName(), term.value().typeName());
    QCOMPARE(t.value().toDateTime(), term.value().toDateTime());
    QCOMPARE(t.value(), term.value());
    QCOMPARE(t.property(), term.property());
}


void QuerySerializationTest::testCustomOptions()
{
    Query query;
    query.addType("File");
    query.addCustomOption("includeFolders", "/home/vishesh/");
    query.addCustomOption("op1", 5);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QVariantMap options = q.customOptions();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options.value("includeFolders"), QVariant("/home/vishesh/"));
    QCOMPARE(options.value("op1"), QVariant(5));

    QCOMPARE(query, q);
}


QTEST_KDEMAIN_CORE(QuerySerializationTest)
