/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "dbversion.h"
#include <QTest>
#include <utility>

namespace Baloo
{

class DbVersionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValid();
    void testReadWrite();
    void testCompareSelf();
    void testCompare();
    void testCompare_data();
    void testDebug();
};

void DbVersionTest::testValid()
{
    QVERIFY(DbVersion::currentDbVersion().isValid());
    QVERIFY(!DbVersion().isValid());
}

void DbVersionTest::testReadWrite()
{
    const auto current = DbVersion::currentDbVersion();

    // Reading/Writing of the current DB version is always OK
    QVERIFY(current.canRead());
    QVERIFY(current.canWrite());

    QVERIFY(!DbVersion{}.canRead());
    QVERIFY(!DbVersion{}.canWrite());
}

void DbVersionTest::testCompareSelf()
{
    const auto current = DbVersion::currentDbVersion();

    QVERIFY(std::is_eq(current <=> current));
    QVERIFY(current == current);
    QVERIFY(std::is_eq(DbVersion{} <=> DbVersion{}));
    QVERIFY(DbVersion{} == DbVersion{});
}

void DbVersionTest::testCompare()
{
    using pii = std::pair<int, int>;
    QFETCH(pii, pa);
    QFETCH(pii, pb);

    const DbVersion a{pa.first, pa.second};
    const DbVersion b{pb.first, pb.second};

    QVERIFY(a == a);
    QVERIFY(a >= a);
    QVERIFY(a <= a);
    QVERIFY(b == b);
    QVERIFY(b >= b);
    QVERIFY(b <= b);

    QVERIFY(a > b);
    QVERIFY(a >= b);
    QVERIFY(b <= a);
    QVERIFY(b < a);
}

void DbVersionTest::testCompare_data()
{
    using pii = std::pair<int, int>;
    QTest::addColumn<std::pair<int, int>>("pa");
    QTest::addColumn<std::pair<int, int>>("pb");

    QTest::addRow("1.0 > 0.0") << pii{1, 0} << pii{0, 0};
    QTest::addRow("0.1 > 0.0") << pii{0, 1} << pii{0, 0};
    QTest::addRow("2.5 > 2.4") << pii{2, 5} << pii{2, 4};
}

void DbVersionTest::testDebug()
{
    QString out;
    QDebug dbg(&out);

    dbg.nospace() << DbVersion{};
    QCOMPARE(out, QStringLiteral("<unknown>"));
    out.clear();

    const DbVersion a{1, 2};
    dbg.nospace() << a;
    QCOMPARE(out, QStringLiteral("(1.2)"));
}

} // namespace Baloo

QTEST_MAIN(Baloo::DbVersionTest)

#include "dbversiontest.moc"
