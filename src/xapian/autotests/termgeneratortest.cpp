/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "termgeneratortest.h"
#include "../termgenerator.h"

#include <QTest>
#include <QDebug>

using namespace Baloo;

void TermGeneratorTest::testWordBoundaries()
{
    QString str("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    Xapian::Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words;
    for (auto it = doc.termlist_begin(); it != doc.termlist_end(); it++) {
        std::string str = *it;
        words << QString::fromUtf8(str.c_str(), str.length());
    }

    QCOMPARE(words[0], QString("32.3"));
    QCOMPARE(words[1], QString("brown"));
    QCOMPARE(words[2], QString("can't"));
    QCOMPARE(words[3], QString("feet"));
    QCOMPARE(words[4], QString("fox"));
    QCOMPARE(words[5], QString("jump"));
    QCOMPARE(words[6], QString("no"));
    QCOMPARE(words[7], QString("quick"));
    QCOMPARE(words[8], QString("right"));
    QCOMPARE(words[9], QString("the"));
    QCOMPARE(words[10], QString("txt"));
    QCOMPARE(words[11], QString("wrong"));
    QCOMPARE(words[12], QString("xx"));
    QCOMPARE(words.size(), 13);
}

QTEST_MAIN(TermGeneratorTest)

#include "termgeneratortest.moc"
