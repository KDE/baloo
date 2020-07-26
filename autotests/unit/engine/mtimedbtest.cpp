/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

        QCOMPARE(db.get(4), QVector<quint64>());
        QCOMPARE(db.get(6), QVector<quint64>());
    }

    void testIter() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(5, 1);
        db.put(6, 2);
        db.put(6, 3);
        db.put(7, 4);
        db.put(8, 5);
        db.put(9, 6);

        PostingIterator* it = db.iterRange(6, std::numeric_limits<quint32>::max());
        QVERIFY(it);

        QVector<quint64> result = {2, 3, 4, 5, 6};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it = db.iterRange(0, 10);
        QVERIFY(it);

        result = {1, 2, 3, 4, 5, 6};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it = db.iterRange(0, 7);
        QVERIFY(it);

        result = {1, 2, 3, 4};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it = db.iterRange(0, 6);
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

        // Empty range
        it = db.iterRange(4, 4);
        QVERIFY(!it);

        it = db.iterRange(10, 20);
        QVERIFY(!it);
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
            it = db.iterRange(6, std::numeric_limits<quint32>::max());
            QVERIFY(it);

            QVector<quint64> result = {2, 3, 4};
            for (quint64 val : result) {
                QCOMPARE(it->next(), static_cast<quint64>(val));
                QCOMPARE(it->docId(), static_cast<quint64>(val));
            }
        }
    }

    void testBeginOfEpoch() {
        MTimeDB db(MTimeDB::create(m_txn), m_txn);

        db.put(0, 1);
        db.put(0, 2);
        db.put(0, 3);
        db.put(1, 4);

        QCOMPARE(db.get(0), QVector<quint64>({1, 2, 3}));
        db.del(99, 2);
        QCOMPARE(db.get(0), QVector<quint64>({1, 2, 3}));
        QCOMPARE(db.get(1), QVector<quint64>({4}));
        db.del(0, 2);
        QCOMPARE(db.get(0), QVector<quint64>({1, 3}));

        PostingIterator* it = db.iterRange(0, 0);
        QVector<quint64> result;
        while (it->next()) {
            result.append(it->docId());
        }
        QCOMPARE(result, QVector<quint64>({1, 3}));

        it = db.iterRange(1, std::numeric_limits<quint32>::max());
        QVERIFY(it->next());
        QCOMPARE(it->docId(), 4);
    }
};

QTEST_MAIN(MTimeDBTest)

#include "mtimedbtest.moc"
