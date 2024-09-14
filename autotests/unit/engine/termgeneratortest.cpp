/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "termgenerator.h"
#include "document.h"

#include <QTest>

using namespace Baloo;

#include <QObject>

class Baloo::TermGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWordBoundaries();
    void testWordBoundariesCJK();
    void testWordBoundariesCJKMixed();
    void testUnderscoreWord();
    void testUnderscore_splitting();
    void testAccentCharacters();
    void testUnicodeCompatibleComposition();
    void testUnicodeLowering();
    void testEmails();
    void testWordPositions();
    void testWordPositionsCJK();
    void testNumbers();
    void testNumbers_data();
    void testControlCharacter();
    void testFilePaths();
    void testFilePaths_data();
    void testApostroph();
    void testApostroph_data();

    QList<QByteArray> allWords(const QString& str)
    {
        Document doc;
        TermGenerator termGen(doc);
        termGen.indexText(str);

        return doc.m_terms.keys();
    }
};

void TermGeneratorTest::testWordBoundaries()
{
    QString str = QString::fromLatin1("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("32.3") << QByteArray("brown") << QByteArray("can't") << QByteArray("feet") << QByteArray("fox") << QByteArray("jump")
                  << QByteArray("no") << QByteArray("quick") << QByteArray("right") << QByteArray("the") << QByteArray("txt") << QByteArray("wrong")
                  << QByteArray("xx");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordBoundariesCJK()
{
    QString str = QString::fromUtf8("你");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("你");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordBoundariesCJKMixed()
{
    // This is a English and CJK mixed string.
    QString str = QString::fromUtf8("hello world!你好世界貴方元気켐ㅇㄹ？☺");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("world") << QByteArray("☺") << QByteArray("你好世界貴方元気") << QByteArray("켐ᄋᄅ");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscoreWord()
{
    QString str = QString::fromLatin1("_plant");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("plant");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscore_splitting()
{
    QString str = QString::fromLatin1("Hello_Howdy");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("howdy");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testAccentCharacters()
{
    QString str = QString::fromUtf8("Como est\xC3\xA1 K\xC3\xBBg"); // "Como está Kûg"

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("como") << QByteArray("esta") << QByteArray("kug");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnicodeCompatibleComposition()
{
    // The 0xfb00 corresponds to U+FB00 which is a 'ff' ligature
    QString expected = QLatin1String("maffab");
    QString str = QLatin1String("ma") + QChar(0xfb00) + QStringLiteral("ab");

    QList<QByteArray> words = allWords(str);
    QCOMPARE(words.size(), 1);

    QByteArray output = words.first();
    QCOMPARE(expected.toUtf8(), output);
}

void TermGeneratorTest::testUnicodeLowering()
{
    // This string is unicode mathematical italic "Hedge"
    QString str = QString::fromUtf8("\xF0\x9D\x90\xBB\xF0\x9D\x91\x92\xF0\x9D\x91\x91\xF0\x9D\x91\x94\xF0\x9D\x91\x92");

    QList<QByteArray> words = allWords(str);

    QCOMPARE(words, {QByteArray("hedge")});
}

void TermGeneratorTest::testEmails()
{
    QString str = QString::fromLatin1("me@vhanda.in");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("in") << QByteArray("me") << QByteArray("vhanda");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordPositions()
{
    Document doc;
    TermGenerator termGen(doc);

    QString str = QString::fromLatin1("Hello hi how hi");
    termGen.indexText(str);

    QList<QByteArray> words = doc.m_terms.keys();

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("hi") << QByteArray("how");
    QCOMPARE(words, expectedWords);

    QList<uint> posInfo1 = doc.m_terms.value("hello").positions;
    QCOMPARE(posInfo1, QList<uint>() << 1);

    QList<uint> posInfo2 = doc.m_terms.value("hi").positions;
    QCOMPARE(posInfo2, QList<uint>() << 2 << 4);

    QList<uint> posInfo3 = doc.m_terms.value("how").positions;
    QCOMPARE(posInfo3, QList<uint>() << 3);
}

void TermGeneratorTest::testWordPositionsCJK()
{
    Document doc;
    TermGenerator termGen(doc);

    // This is a Chinese sentence: Hello! I know about you.
    QString str = QString::fromUtf8("你好你好！我认识你。");
    termGen.indexText(str);

    QList<QByteArray> words = doc.m_terms.keys();
    QList<QByteArray> expectedWords;
    // Full width question mark is split point, and the fullwidth period is trimmed.
    expectedWords << QByteArray("你好你好") << QByteArray("我认识你");
    QCOMPARE(words, expectedWords);

    QList<uint> posInfo1 = doc.m_terms.value("你好你好").positions;
    QCOMPARE(posInfo1, QList<uint>() << 1);

    QList<uint> posInfo2 = doc.m_terms.value("我认识你").positions;
    QCOMPARE(posInfo2, QList<uint>() << 2);
}

void TermGeneratorTest::testNumbers()
{
    QFETCH(QString, input);
    QFETCH(QList<QByteArray>, expectedWords);

    auto words = allWords(input);
    QEXPECT_FAIL("Negative Integer", "Minus signs not handled correctly", Continue);
    QEXPECT_FAIL("Negative Real", "Minus signs not handled correctly", Continue);
    QEXPECT_FAIL("Negative Integer Scientific Notation", "Minus signs not handled correctly", Continue);
    QEXPECT_FAIL("Negative Real Scientific Notation", "Minus signs not handled correctly", Continue);
    QEXPECT_FAIL("Scientific Notation with Negative Exponent", "Split on the minus in the Negative Exponent", Continue);
    QEXPECT_FAIL("Comma as decimal separator", "Comma separated integers treated as a real", Continue);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testNumbers_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<QByteArray>>("expectedWords");

    QTest::addRow("Positive Integer") << QStringLiteral("1230") << QList<QByteArray>({"1230"});
    QTest::addRow("Negative Integer") << QStringLiteral("-1230") << QList<QByteArray>({"-1230"});
    QTest::addRow("Positive Real") << QStringLiteral("1230.0") << QList<QByteArray>({"1230.0"});
    QTest::addRow("Negative Real") << QStringLiteral("-1230.0") << QList<QByteArray>({"-1230.0"});
    QTest::addRow("Positive Integer Scientific Notation") << QStringLiteral("1230e0") << QList<QByteArray>({"1230e0"});
    QTest::addRow("Negative Integer Scientific Notation") << QStringLiteral("-1230e0") << QList<QByteArray>({"-1230e0"});
    QTest::addRow("Positive Real Scientific Notation") << QStringLiteral("1.23e4") << QList<QByteArray>({"1.23e4"});
    QTest::addRow("Negative Real Scientific Notation") << QStringLiteral("-1.23e4") << QList<QByteArray>({"-1.23e4"});
    QTest::addRow("Scientific Notation with Negative Exponent") << QStringLiteral("1.23e-1") << QList<QByteArray>({"1.23e-1"});

    // Correct interpretation of `12,34` depends on the context. For a CSV file, it likely denotes a field separator (thus splitting should
    // be done), while in many european countries it might be used for a decimal separator (i.e. denoting the value `1234/100 == 12.34`).
    // '1,234' has the same ambiguity, but additionally in an english text it might be used as a  thousands separator (i.e. denoting
    // the same value as 1234).
    // Other ambigous strings: "1.234,567" (two values in CSV and english (1.234, 567), but one number (1234.56) elsewhere
    // "1,234.567, 1,234,567.890": two numbers in english text, four values in CSV
    QTest::addRow("Comma as decimal separator") << QStringLiteral("12,34") << QList<QByteArray>({"12", "34"});
    QTest::addRow("Comma as thousands separator") << QStringLiteral("1,234") << QList<QByteArray>({"1,234"});
    QTest::addRow("Comma as word separator") << QStringLiteral("twelve,thirtyfour") << QList<QByteArray>({"thirtyfour", "twelve"});
}

void TermGeneratorTest::testControlCharacter()
{
    QString str = QString::fromUtf8("word1\u0001word2");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords = { "word1", "word2" };

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testFilePaths()
{
    QFETCH(QString, input);
    QFETCH(QList<QByteArray>, expectedWords);

    auto words = allWords(input);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testFilePaths_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<QByteArray>>("expectedWords");

    QTest::addRow("filename with suffix") << QStringLiteral("file.png")
        << QList<QByteArray>({"file", "png"});
    QTest::addRow("filename") << QStringLiteral("foo_bar.png")
        << QList<QByteArray>({"bar", "foo", "png"});
    QTest::addRow("filepath") << QStringLiteral("/foo/bar")
        << QList<QByteArray>({"bar", "foo"});
}

void TermGeneratorTest::testApostroph()
{
    QFETCH(QString, input);
    QFETCH(QList<QByteArray>, expectedWords);

    auto words = allWords(input);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testApostroph_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<QByteArray>>("expectedWords");

    QTest::addRow("Leading") << QStringLiteral("'one two")
        << QList<QByteArray>({"one", "two"});
    QTest::addRow("Middle") << QStringLiteral("one'two three")
        << QList<QByteArray>({"one'two", "three"});
    QTest::addRow("End") << QStringLiteral("one' two")
        << QList<QByteArray>({"one", "two"});
}

QTEST_MAIN(TermGeneratorTest)

#include "termgeneratortest.moc"
