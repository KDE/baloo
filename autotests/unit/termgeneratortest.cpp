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

#include "termgenerator.h"
#include "document.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

#include <QObject>

class Baloo::TermGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWordBoundaries();
    void testUnderscore_splitting();
    void testAccetCharacters();
    void testUnicodeCompatibleComposition();
    void testEmails();
    void testWordPositions();

    QList<QByteArray> allWords(const Document& doc)
    {
        return doc.m_terms.keys();
    }
};

void TermGeneratorTest::testWordBoundaries()
{
    QString str = QString::fromLatin1("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QList<QByteArray> words = allWords(doc);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("32.3") << QByteArray("brown") << QByteArray("can't") << QByteArray("feet") << QByteArray("fox") << QByteArray("jump")
                  << QByteArray("no") << QByteArray("quick") << QByteArray("right") << QByteArray("the") << QByteArray("txt") << QByteArray("wrong")
                  << QByteArray("xx");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscore_splitting()
{
    QString str = QString::fromLatin1("Hello_Howdy");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QList<QByteArray> words = allWords(doc);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("howdy");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testAccetCharacters()
{
    QString str = QString::fromLatin1("Como está Kûg");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QList<QByteArray> words = allWords(doc);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("como") << QByteArray("esta") << QByteArray("kug");

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

    QList<QByteArray> words = allWords(doc);
    QCOMPARE(words.size(), 1);

    QByteArray output = words.first();
    QCOMPARE(str.toUtf8(), output);
}

void TermGeneratorTest::testEmails()
{
    QString str = QString::fromLatin1("me@vhanda.in");

    Document doc;
    TermGenerator termGen(&doc);
    termGen.indexText(str);

    QList<QByteArray> words = allWords(doc);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("in") << QByteArray("me") << QByteArray("vhanda");

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
    QCOMPARE(*it, (quint64)1);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "hi");
    end = xap->positionlist_end(1, "hi");
    QVERIFY(it != end);
    QCOMPARE(*it, (quint64)2);
    it++;
    QCOMPARE(*it, (quint64)4);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "how");
    end = xap->positionlist_end(1, "how");
    QVERIFY(it != end);
    QCOMPARE(*it, (quint64)3);
    it++;
    QVERIFY(it == end);
    */
}

QTEST_MAIN(TermGeneratorTest)

#include "termgeneratortest.moc"
