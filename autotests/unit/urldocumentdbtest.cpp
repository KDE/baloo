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

#include "urldocumentdb.h"
#include "singledbtest.h"

#include <QVector>

using namespace Baloo;

class UrlDocumentDBTest : public SingleDBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        UrlDocumentDB db(m_txn);

        QByteArray arr = "/home/blah";
        db.put(arr, 1);

        QCOMPARE(db.get(arr), static_cast<uint>(1));

        db.del(arr);
        QCOMPARE(db.get(arr), static_cast<uint>(0));
    }

    void testIter() {
        UrlDocumentDB db(m_txn);

        db.put("/home/v", 1);
        db.put("/home/v1", 2);
        db.put("/home/v2", 3);
        db.put("/home/b", 4);
        db.put("/home/w", 5);

        PostingIterator* it = db.prefixIter("/home/v");
        QVERIFY(it);

        QVector<uint> result = {1, 2, 3};
        for (uint val : result) {
            QCOMPARE(it->next(), static_cast<uint>(val));
            QCOMPARE(it->docId(), static_cast<uint>(val));
        }

        it = db.prefixIter("/home/w");
        QVERIFY(it);

        result = {5};
        for (uint val : result) {
            QCOMPARE(it->next(), static_cast<uint>(val));
            QCOMPARE(it->docId(), static_cast<uint>(val));
        }

        it = db.prefixIter("/home/b");
        QVERIFY(it);

        result = {4};
        for (uint val : result) {
            QCOMPARE(it->next(), static_cast<uint>(val));
            QCOMPARE(it->docId(), static_cast<uint>(val));
        }

        it = db.prefixIter("/home/f");
        QVERIFY(it == 0);
    }
};

QTEST_MAIN(UrlDocumentDBTest)

#include "urldocumentdbtest.moc"
