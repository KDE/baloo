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

#include "idtreedb.h"
#include "singledbtest.h"
#include "postingiterator.h"

using namespace Baloo;

class BALOO_ENGINE_EXPORT IdTreeDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        IdTreeDB db(m_txn);

        QVector<quint64> val = {5, 6, 7};
        db.put(1, val);

        QCOMPARE(db.get(1), val);

        db.del(1);
        QCOMPARE(db.get(1), QVector<quint64>());
    }

    void testIter() {
        IdTreeDB db(m_txn);

        db.put(1, {5, 6, 7, 8});
        db.put(6, {9, 11, 19});
        db.put(8, {13, 15});
        db.put(13, {18});

        PostingIterator* it = db.iter(1);
        QVERIFY(it);

        QVector<quint64> result = {1, 5, 6, 7, 8, 9, 11, 13, 15, 18, 19};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }
};

QTEST_MAIN(IdTreeDBTest)

#include "idtreedbtest.moc"
