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

#include "indexingleveldb.h"
#include "singledbtest.h"

using namespace Baloo;

class IndexingLevelDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testFetchItems();
};

void IndexingLevelDBTest::test()
{
    IndexingLevelDB db(m_txn);

    QCOMPARE(db.contains(1), false);
    db.put(1);
    QCOMPARE(db.contains(1), true);

    db.del(1);
    QCOMPARE(db.contains(1), false);
}

void IndexingLevelDBTest::testFetchItems()
{
    IndexingLevelDB db(m_txn);

    db.put(1);
    db.put(6);
    db.put(8);

    QVector<uint> acVec = db.fetchItems(10);
    QVector<uint> exVec = {1, 6, 8};

    QCOMPARE(acVec, exVec);
}

QTEST_MAIN(IndexingLevelDBTest)

#include "indexingleveldbtest.moc"
