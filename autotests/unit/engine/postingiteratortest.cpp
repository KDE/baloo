/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "vectorpostingiterator.h"

#include <QTest>

using namespace Baloo;

class PostingIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void test2();
};

void PostingIteratorTest::test()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {2, 3, 4, 9, 11};
    QVector<quint64> l3 = {4, 7};

    std::vector<std::unique_ptr<PostingIterator>> orvec;
    orvec.push_back(std::make_unique<VectorPostingIterator>(l2));
    orvec.push_back(std::make_unique<VectorPostingIterator>(l3));

    // {l1} AND ({l2} OR {l3})
    std::vector<std::unique_ptr<PostingIterator>> andvec;
    andvec.push_back(std::make_unique<VectorPostingIterator>(l1));
    andvec.push_back(std::make_unique<OrPostingIterator>(std::move(orvec)));

    AndPostingIterator it(std::move(andvec));
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    QVector<quint64> result = {3, 7};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

void PostingIteratorTest::test2()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {2, 3, 4, 9, 11};
    QVector<quint64> l3 = {3, 7};

    std::vector<std::unique_ptr<PostingIterator>> orvec;
    orvec.push_back(std::make_unique<VectorPostingIterator>(l2));
    orvec.push_back(std::make_unique<VectorPostingIterator>(l3));

    // {l1} AND ({l2} OR {l3})
    std::vector<std::unique_ptr<PostingIterator>> andvec;
    andvec.push_back(std::make_unique<VectorPostingIterator>(l1));
    andvec.push_back(std::make_unique<OrPostingIterator>(std::move(orvec)));

    AndPostingIterator it(std::move(andvec));
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    QVector<quint64> result = {3, 7};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

QTEST_MAIN(PostingIteratorTest)

#include "postingiteratortest.moc"
