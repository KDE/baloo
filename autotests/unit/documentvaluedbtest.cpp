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

#include "documentvaluedb.h"
#include "singledbtest.h"

using namespace Baloo;

class DocumentValueDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void DocumentValueDBTest::test()
{
    DocumentValueDB db(m_txn);
    QByteArray arr1("foo");
    QByteArray arr2("goo");

    db.put(1, 0, arr1);
    db.put(1, 6, arr2);
    db.put(2, 4, arr1);

    QCOMPARE(db.get(1, 0), arr1);
    QCOMPARE(db.get(1, 6), arr2);
    QCOMPARE(db.get(2, 4), arr1);

    db.del(1);
    QCOMPARE(db.get(1, 0), QByteArray());
    QCOMPARE(db.get(1, 6), QByteArray());
    QCOMPARE(db.get(2, 4), arr1);
}

QTEST_MAIN(DocumentValueDBTest)

#include "documentvaluedbtest.moc"
