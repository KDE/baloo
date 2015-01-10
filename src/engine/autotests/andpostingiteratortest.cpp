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

#include "andpostingiterator.h"
#include "vectorpostingiterator.h"

#include <QTest>

using namespace Baloo;

class AndPostingIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void AndPostingIteratorTest::test()
{
    QVector<uint> l1 = {1, 3, 5, 7};
    QVector<uint> l2 = {3, 4, 5, 7, 9, 11};
    QVector<uint> l3 = {1, 3, 7};

    VectorPostingIterator it1(l1);
    VectorPostingIterator it2(l2);
    VectorPostingIterator it3(l3);

    QVector<PostingIterator*> vec = {&it1, &it2, &it3};
    AndPostingIterator it(vec);
    QCOMPARE(it.docId(), static_cast<uint>(0));

    QVector<uint> result = {3, 7};
    for (uint val : result) {
        QCOMPARE(it.next(), static_cast<uint>(val));
        QCOMPARE(it.docId(), static_cast<uint>(val));
    }
    QCOMPARE(it.next(), static_cast<uint>(0));
    QCOMPARE(it.docId(), static_cast<uint>(0));
}


QTEST_MAIN(AndPostingIteratorTest)

#include "andpostingiteratortest.moc"
