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

#include "../andpostinglist.h"

#include <QTest>

using namespace Baloo;

class AndPostingListTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void AndPostingListTest::test()
{
    PostingList l1 = {1, 3, 5, 7};
    PostingList l2 = {3, 4, 5, 9, 11};

    AndPostingList l(l1, l2);
    l.compute();

    PostingList result = {3, 5};
    QCOMPARE(l.result(), result);
}


QTEST_MAIN(AndPostingListTest)

#include "andpostinglisttest.moc"
