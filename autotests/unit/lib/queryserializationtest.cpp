/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "query.h"
#include "term.h"

#include <QTest>

using namespace Baloo;

class QuerySerializationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBasic();
    void testJSON();
    void testJSON_data();
    void testURL();
    void testURL_data();

    void testCustomOptions();
};

// Test a simple query with no terms
void QuerySerializationTest::testBasic()
{
    Query query;
    query.setLimit(5);
    query.setOffset(1);
    query.setSearchString(QStringLiteral("Bookie"));
    query.addType(QStringLiteral("File/Audio"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q.limit(), static_cast<uint>(5));
    QCOMPARE(q.offset(), static_cast<uint>(1));
    QCOMPARE(q.searchString(), QLatin1String("Bookie"));
    QCOMPARE(q.types().size(), 2);
    QVERIFY(q.types().contains(QLatin1String("File")));
    QVERIFY(q.types().contains(QLatin1String("Audio")));

    QCOMPARE(q, query);
}

void QuerySerializationTest::testJSON()
{
    QFETCH(QString, searchString);

    Query query;
    query.setSearchString(searchString);

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testJSON_data()
{
    QTest::addColumn<QString>("searchString");

    QTest::addRow("term") << QStringLiteral("prop:value");
    QTest::addRow("andTerm") << QStringLiteral("prop1:1 AND prop2:2");
    QTest::addRow("dateTerm") << QStringLiteral("prop:2015-05-01");
    QTest::addRow("dateTimeTerm") << QStringLiteral("prop:2015-05-01T23:44:11");
}

void QuerySerializationTest::testURL()
{
    QFETCH(QString, searchString);
    QString title = QString::fromUtf8(QTest::currentDataTag());

    Query query;
    query.setSearchString(searchString);

    const auto url = query.toSearchUrl(title);
    QCOMPARE(Query::titleFromQueryUrl(url), title);

    Query q = Query::fromSearchUrl(url);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testURL_data()
{
    QTest::addColumn<QString>("searchString");

    QTest::addRow("term") << QStringLiteral("prop:value");
    QTest::addRow("andTerm") << QStringLiteral("prop1:1 AND prop2:2");
    QTest::addRow("dateTerm") << QStringLiteral("prop:2015-05-01");
    QTest::addRow("dateTimeTerm") << QStringLiteral("prop:2015-05-01T23:44:11");
}

void QuerySerializationTest::testCustomOptions()
{
    Query query;
    query.addType(QStringLiteral("File"));
    query.setIncludeFolder(QStringLiteral("/home/vishesh"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QString includeFolder = q.includeFolder();
    QCOMPARE(includeFolder, QStringLiteral("/home/vishesh"));

    QCOMPARE(query, q);
}

QTEST_MAIN(QuerySerializationTest)

#include "queryserializationtest.moc"
