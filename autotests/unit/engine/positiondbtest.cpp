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

#include "positiondb.h"
#include "positioninfo.h"
#include "postingiterator.h"
#include "singledbtest.h"

#include <QRegularExpression>

using namespace Baloo;

class PositionDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        QByteArray word("fire");
        PositionInfo pos1;
        pos1.docId = 1;
        pos1.positions = QVector<uint>() << 1 << 5 << 6;

        PositionInfo pos2;
        pos2.docId = 5;
        pos2.positions = QVector<uint>() << 41 << 96 << 116;

        QVector<PositionInfo> list = {pos1, pos2};

        db.put(word, list);
        QVector<PositionInfo> res = db.get(word);
        QCOMPARE(res, list);
    }

    void testIter() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        QByteArray word("fire");
        PositionInfo pos1;
        pos1.docId = 1;
        pos1.positions = QVector<uint>() << 1 << 5 << 6;

        PositionInfo pos2;
        pos2.docId = 5;
        pos2.positions = QVector<uint>() << 41 << 96 << 116;

        QVector<PositionInfo> list = {pos1, pos2};

        db.put(word, list);

        PostingIterator* it = db.iter(word);
        QCOMPARE(it->docId(), static_cast<quint64>(0));
        QVERIFY(it->positions().isEmpty());

        QCOMPARE(it->next(), static_cast<quint64>(1));
        QCOMPARE(it->docId(), static_cast<quint64>(1));
        QCOMPARE(it->positions(), pos1.positions);

        QCOMPARE(it->next(), static_cast<quint64>(5));
        QCOMPARE(it->docId(), static_cast<quint64>(5));
        QCOMPARE(it->positions(), pos2.positions);

        QCOMPARE(it->next(), static_cast<quint64>(0));
        QCOMPARE(it->docId(), static_cast<quint64>(0));
        QVERIFY(it->positions().isEmpty());
    }

    void testPrefixIter() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        db.put("abc", {PositionInfo(1), PositionInfo(4), PositionInfo(5), PositionInfo(9), PositionInfo(11)});
        db.put("fir", {PositionInfo(1), PositionInfo(3), PositionInfo(5)});
        db.put("fire", {PositionInfo(1), PositionInfo(8), PositionInfo(9)});
        db.put("fore", {PositionInfo(2), PositionInfo(3), PositionInfo(5)});

        PostingIterator* it = db.prefixIter("fi");
        QVERIFY(it);

        QVector<quint64> result = {1, 3, 5, 8, 9};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
            QVERIFY(it->positions().isEmpty());
        }
    }

    void testRegExpIter() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        db.put("abc", {PositionInfo(1), PositionInfo(4), PositionInfo(5), PositionInfo(9), PositionInfo(11)});
        db.put("fir", {PositionInfo(1), PositionInfo(3), PositionInfo(5), PositionInfo(7)});
        db.put("fire", {PositionInfo(1), PositionInfo(8)});
        db.put("fore", {PositionInfo(2), PositionInfo(3), PositionInfo(5)});
        db.put("zib", {PositionInfo(4), PositionInfo(5), PositionInfo(6)});

        PostingIterator* it = db.regexpIter(QRegularExpression(".re"), QByteArray("f"));
        QVERIFY(it);

        QVector<quint64> result = {1, 2, 3, 5, 8};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        // Non existing
        it = db.regexpIter(QRegularExpression("dub"), QByteArray("f"));
        QVERIFY(it == 0);
    }

    void testCompIter() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        db.put("abc", {PositionInfo(1), PositionInfo(4), PositionInfo(5), PositionInfo(9), PositionInfo(11)});
        db.put("R1", {PositionInfo(1), PositionInfo(3), PositionInfo(5), PositionInfo(7)});
        db.put("R2", {PositionInfo(1), PositionInfo(8)});

        PostingIterator* it = db.compIter("R", "2", PositionDB::GreaterEqual);
        QVERIFY(it);

        QVector<quint64> result = {1, 2, 3, 5, 8};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it = db.compIter("R", "2", PositionDB::LessEqual);
        QVERIFY(it);
        result = {1, 3, 5, 7, 8};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testFetchTermsStartingWith() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        db.put("abc", {PositionInfo(1), PositionInfo(4), PositionInfo(5), PositionInfo(9), PositionInfo(11)});
        db.put("fir", {PositionInfo(1), PositionInfo(3), PositionInfo(5), PositionInfo(7)});
        db.put("fire", {PositionInfo(1), PositionInfo(8)});
        db.put("fore", {PositionInfo(2), PositionInfo(3), PositionInfo(5)});
        db.put("zib", {PositionInfo(4), PositionInfo(5), PositionInfo(6)});

        QVector<QByteArray> list = {"fir", "fire", "fore"};
        QCOMPARE(db.fetchTermsStartingWith("f"), list);
    }
};

QTEST_MAIN(PositionDBTest)

#include "positiondbtest.moc"
