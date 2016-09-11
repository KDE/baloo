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

#include "documenttimedb.h"
#include "singledbtest.h"

using namespace Baloo;

class DocumentTimeDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testAllowZeroTime();
};

void DocumentTimeDBTest::test()
{
    DocumentTimeDB db(DocumentTimeDB::create(m_txn), m_txn);

    DocumentTimeDB::TimeInfo info;
    info.mTime = 5;
    info.cTime = 6;

    db.put(1, info);
    QCOMPARE(db.get(1), info);

    db.del(1);
    QCOMPARE(db.get(1), DocumentTimeDB::TimeInfo());
}

void DocumentTimeDBTest::testAllowZeroTime()
{
    DocumentTimeDB db(DocumentTimeDB::create(m_txn), m_txn);

    // we must be able to handle zero time, aka 1970...
    DocumentTimeDB::TimeInfo info;
    info.mTime = 0;
    info.cTime = 0;

    db.put(1, info);
    QCOMPARE(db.get(1), info);

    db.del(1);
    QCOMPARE(db.get(1), DocumentTimeDB::TimeInfo());
}

QTEST_MAIN(DocumentTimeDBTest)

#include "documenttimedbtest.moc"
