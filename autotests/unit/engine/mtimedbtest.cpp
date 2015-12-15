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
#include "postingiterator.h"
#include "singledbtest.h"

using namespace Baloo;

class MTimeDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        QCOMPARE(db.get(5), QVector<quint64>() << 1);
        db.del(5, 1);
        QCOMPARE(db.get(5), QVector<quint64>());
    }

    void testMultiple() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        db.put(5, 2);
        db.put(5, 3);

        QCOMPARE(db.get(5), QVector<quint64>() << 1 << 2 << 3);
        db.del(5, 2);
        QCOMPARE(db.get(5), QVector<quint64>() << 1 << 3);
    }

    void testIter() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        db.put(6, 2);
        db.put(6, 3);
        db.put(7, 4);
        db.put(8, 5);
        db.put(9, 6);

        PostingIterator* it = db.iter(6, MTimeDB::GreaterEqual);
        QVERIFY(it);

        QVector<quint64> result = {2, 3, 4, 5, 6};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it = db.iter(7, MTimeDB::LessEqual);
        QVERIFY(it);

        result = {1, 2, 3};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testRangeIter() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        db.put(6, 2);
        db.put(6, 3);
        db.put(7, 4);
        db.put(8, 5);
        db.put(9, 6);

        PostingIterator* it = db.iterRange(6, 8);
        QVERIFY(it);

        QVector<quint64> result = {2, 3, 4, 5};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testSortedAndUnique()
    {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        db.put(6, 4);
        db.put(6, 2);
        db.put(6, 3);
        db.put(7, 3);

        QCOMPARE(db.get(6), QVector<quint64>() << 2 << 3 << 4);

        PostingIterator* it = db.iterRange(5, 7);
        QVERIFY(it);

        {
            QVector<quint64> result = {1, 2, 3, 4};
            for (quint64 val : result) {
                QCOMPARE(it->next(), static_cast<quint64>(val));
                QCOMPARE(it->docId(), static_cast<quint64>(val));
            }
        }

        {
            it = db.iter(6, MTimeDB::GreaterEqual);
            QVERIFY(it);

            QVector<quint64> result = {2, 3, 4};
            for (quint64 val : result) {
                QCOMPARE(it->next(), static_cast<quint64>(val));
                QCOMPARE(it->docId(), static_cast<quint64>(val));
            }
        }
    }
};

QTEST_MAIN(MTimeDBTest)

#include "mtimedbtest.moc"
