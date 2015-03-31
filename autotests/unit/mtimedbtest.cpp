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

#include "mtimedb.h"
#include "singledbtest.h"

using namespace Baloo;

class MTimeDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        MTimeDB db(m_txn);

        db.put(5, 1);
        QCOMPARE(db.get(5), QVector<quint64>() << 1);
        db.del(5, 1);
        QCOMPARE(db.get(5), QVector<quint64>());
    }

    void testMultiple() {
        MTimeDB db(m_txn);

        db.put(5, 1);
        db.put(5, 2);
        db.put(5, 3);

        QCOMPARE(db.get(5), QVector<quint64>() << 1 << 2 << 3);
        db.del(5, 2);
        QCOMPARE(db.get(5), QVector<quint64>() << 1 << 3);
    }
};

QTEST_MAIN(MTimeDBTest)

#include "mtimedbtest.moc"
