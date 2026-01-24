/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

    std::vector<std::unique_ptr<PostingIterator>> vec;

    vec.push_back(std::make_unique<VectorPostingIterator>(l1));
    vec.push_back(std::make_unique<VectorPostingIterator>(l2));
    vec.push_back(std::make_unique<VectorPostingIterator>(l3));

    AndPostingIterator it(std::move(vec));
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

    std::vector<std::unique_ptr<PostingIterator>> vec;

    vec.push_back(std::make_unique<VectorPostingIterator>(l1));
    vec.push_back(nullptr);
    vec.push_back(std::make_unique<VectorPostingIterator>(l2));

    AndPostingIterator it(std::move(vec));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

QTEST_MAIN(AndPostingIteratorTest)

#include "andpostingiteratortest.moc"
