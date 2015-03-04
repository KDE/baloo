/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "documentiddb.h"
#include "singledbtest.h"

using namespace Baloo;

class DocIdDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testFetchItems();

    void testSize()
    {
        DocumentIdDB db(m_txn);

        db.put(1);
        db.put(6);
        db.put(8);
        QCOMPARE(db.size(), static_cast<uint>(3));
    }
};

void DocIdDBTest::test()
{
    DocumentIdDB db(m_txn);

    QCOMPARE(db.contains(1), false);
    db.put(1);
    QCOMPARE(db.contains(1), true);

    db.del(1);
    QCOMPARE(db.contains(1), false);
}

void DocIdDBTest::testFetchItems()
{
    DocumentIdDB db(m_txn);

    db.put(1);
    db.put(6);
    db.put(8);

    QVector<quint64> acVec = db.fetchItems(10);
    QVector<quint64> exVec = {1, 6, 8};

    QCOMPARE(acVec, exVec);
}

QTEST_MAIN(DocIdDBTest)

#include "documentiddbtest.moc"
