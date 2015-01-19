/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2014-2015 Vishesh Handa <vhanda@kde.org>
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
#include "termgenerator.h"
#include "document.h"

#include <QTest>
#include <QDebug>
#include <QTemporaryDir>

using namespace Baloo;

namespace {
    QStringList allWords(const Document& doc)
    {
        QStringList words;
        for (const QByteArray& ba : doc.terms()) {
            words << QString::fromUtf8(ba);
        }

        return words;
    }
}
void TermGeneratorTest::testWordBoundaries()
{
    QString str = QString::fromLatin1("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QLatin1String("32.3") << QLatin1String("brown") << QLatin1String("can't") << QLatin1String("feet") << QLatin1String("fox") << QLatin1String("jump")
                  << QLatin1String("no") << QLatin1String("quick") << QLatin1String("right") << QLatin1String("the") << QLatin1String("txt") << QLatin1String("wrong")
                  << QLatin1String("xx");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscore_splitting()
{
    QString str = QString::fromLatin1("Hello_Howdy");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QLatin1String("hello") << QLatin1String("howdy");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testAccetCharacters()
{
    QString str = QString::fromLatin1("Como está Kûg");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QLatin1String("como") << QLatin1String("esta") << QLatin1String("kug");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnicodeCompatibleComposition()
{
    // The 0xfb00 corresponds to U+FB00 which is a 'ff'
    QString str = QLatin1Literal("maffab");
    QString str2 = QLatin1Literal("ma") + QChar(0xfb00) + QStringLiteral("ab");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str2);

    QStringList words = allWords(doc);
    QCOMPARE(words.size(), 1);

    QString output = words.first();
    QCOMPARE(str, output);
}

void TermGeneratorTest::testEmails()
{
    QString str = QString::fromLatin1("me@vhanda.in");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QLatin1String("in") << QLatin1String("me") << QLatin1String("vhanda");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordPositions()
{
    Document doc;
    TermGenerator termGen(&doc);

    QString str = QString::fromLatin1("Hello hi how hi");
    termGen.indexText(str);

    /*
     * FIXME: Verify positional information!
    Xapian::PositionIterator it = xap->positionlist_begin(1, "hello");
    Xapian::PositionIterator end = xap->positionlist_end(1, "hello");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)1);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "hi");
    end = xap->positionlist_end(1, "hi");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)2);
    it++;
    QCOMPARE(*it, (uint)4);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "how");
    end = xap->positionlist_end(1, "how");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)3);
    it++;
    QVERIFY(it == end);
    */
}

QTEST_MAIN(TermGeneratorTest)

