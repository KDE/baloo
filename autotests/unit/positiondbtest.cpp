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
};

QTEST_MAIN(PositionDBTest)

#include "positiondbtest.moc"
