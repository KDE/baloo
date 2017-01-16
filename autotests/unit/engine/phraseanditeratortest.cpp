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

#include "phraseanditerator.h"
#include "vectorpositioninfoiterator.h"
#include "positioninfo.h"

#include <QTest>

using namespace Baloo;

class PhraseAndIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testNullIterators();
};

void PhraseAndIteratorTest::test()
{
    // Term 1
    PositionInfo pi1;
    pi1.docId = 2;
    pi1.positions = {5, 9};

    PositionInfo pi2;
    pi2.docId = 4;
    pi2.positions = {4, 2};

    QVector<PositionInfo> vec1;
    vec1 << pi1 << pi2;

    // Term 2
    PositionInfo pi3;
    pi3.docId = 2;
    pi3.positions = {6, 7};

    PositionInfo pi4;
    pi4.docId = 4;
    pi4.positions = {6, 2};

    PositionInfo pi5;
    pi5.docId = 7;
    pi5.positions = {1, 4, 2};

    QVector<PositionInfo> vec2;
    vec2 << pi3 << pi4 << pi5;

    VectorPositionInfoIterator* it1 = new VectorPositionInfoIterator(vec1);
    VectorPositionInfoIterator* it2 = new VectorPositionInfoIterator(vec2);

    QVector<PostingIterator*> vec = {it1, it2};
    PhraseAndIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    // The Query is "term1 term2". term1 must appear one position before term2
    QVector<quint64> result = {2};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

void PhraseAndIteratorTest::testNullIterators()
{
    // Term 1
    PositionInfo pi1;
    pi1.docId = 2;
    pi1.positions = {5, 9};

    QVector<PositionInfo> vec1;
    vec1 << pi1;

    // Term 2
    PositionInfo pi2;
    pi2.docId = 2;
    pi2.positions = {6, 7};

    QVector<PositionInfo> vec2;
    vec2 << pi2;

    VectorPositionInfoIterator* it1 = new VectorPositionInfoIterator(vec1);
    VectorPositionInfoIterator* it2 = new VectorPositionInfoIterator(vec2);

    QVector<PostingIterator*> vec = {it1, nullptr, it2};
    PhraseAndIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

QTEST_MAIN(PhraseAndIteratorTest)

#include "phraseanditeratortest.moc"
