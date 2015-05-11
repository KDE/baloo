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

#include "documentdb.h"
#include "singledbtest.h"

using namespace Baloo;

class DocumentDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void DocumentDBTest::test()
{
    DocumentDB db(DocumentDB::create("db", m_txn), m_txn);

    QVector<QByteArray> list = {"a", "aab", "abc"};
    db.put(1, list);

    QCOMPARE(db.get(1), list);
}

QTEST_MAIN(DocumentDBTest)

#include "documentdbtest.moc"
