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
    void testNullIterators();
};

void AndPostingIteratorTest::test()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {3, 4, 5, 7, 9, 11};
    QVector<quint64> l3 = {1, 3, 7};

    VectorPostingIterator* it1 = new VectorPostingIterator(l1);
    VectorPostingIterator* it2 = new VectorPostingIterator(l2);
    VectorPostingIterator* it3 = new VectorPostingIterator(l3);

    QVector<PostingIterator*> vec = {it1, it2, it3};
    AndPostingIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    QVector<quint64> result = {3, 7};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

void AndPostingIteratorTest::testNullIterators()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {3, 4, 5, 7, 9, 11};

    VectorPostingIterator* it1 = new VectorPostingIterator(l1);
    VectorPostingIterator* it2 = new VectorPostingIterator(l2);

    QVector<PostingIterator*> vec = {it1, nullptr, it2};

    AndPostingIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}


QTEST_MAIN(AndPostingIteratorTest)

#include "andpostingiteratortest.moc"
