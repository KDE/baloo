/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "document.h"
#include "extractor/result.h"

#include <QTest>

using namespace Baloo;
using namespace Qt::StringLiterals;

namespace
{
struct ValueProperty {
    KFileMetaData::Property::Property property;
    QVariant value;
};

} // namespace <anonymous>

class Baloo::DocumentContentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testResultText();
    void testResultText_data();
    void testResultProperty();
    void testResultProperty_data();
    void benchmarkResultTerms();
    void benchmarkResultTerms_data();
};

void DocumentContentTest::testResultText_data()
{
    using namespace Qt::Literals::StringLiterals;

    QTest::addColumn<QString>("text");
    QTest::addColumn<QByteArrayList>("expectedKeys");

    QTest::addRow("notext") << QString{} << QByteArrayList{};
    QTest::addRow("single") << u"single"_s << QByteArrayList{"single"};
    QTest::addRow("two words") << u"two words"_s << QByteArrayList{"two", "words"};
    QTest::addRow("repeated") << u"a b c b a"_s << QByteArrayList{"a", "b", "c"};
}

void DocumentContentTest::testResultText()
{
    QFETCH(QString, text);
    QFETCH(QByteArrayList, expectedKeys);

    Baloo::Result result;
    result.append(text);
    result.finish();

    auto doc = result.document();
    QCOMPARE(doc.m_terms.keys(), expectedKeys);
}

void DocumentContentTest::testResultProperty_data()
{
    using namespace Qt::Literals::StringLiterals;
    using KFMP = KFileMetaData::Property::Property;

    QTest::addColumn<QList<ValueProperty>>("properties");
    QTest::addColumn<QByteArrayList>("expectedKeys");

    struct TC {
        const char *name;
        QList<ValueProperty> properties;
        QByteArrayList expectedKeys;
    } testCases[] = {
        {"noproperties", {}, {}},
        {"single1word", {{KFMP::Model, u"Model"_s}}, {"X31-model"}},
        {"single2word", {{KFMP::Model, u"Great Camera"_s}}, {"X31-camera", "X31-great"}},
        {"multiple",
         {
             {KFMP::Model, u"Great Camera"_s},
             {KFMP::Manufacturer, u"Vendor"_s},
         },
         {"X30-vendor", "X31-camera", "X31-great"}},
        {"singleContent", {{KFMP::Artist, u"Artist"_s}}, {"X9-artist", "artist"}},
        {"mixedContent",
         {
             {KFMP::Artist, u"Artist Name"_s},
             {KFMP::Model, u"Great Camera"_s},
             {KFMP::Manufacturer, u"Vendor"_s},
         },
         {"X30-vendor", "X31-camera", "X31-great", "X9-artist", "X9-name", "artist", "name"}},
    };

    for (const auto &tc : testCases) {
        QTest::addRow("%s", tc.name) << tc.properties << tc.expectedKeys;
    }
}

void DocumentContentTest::testResultProperty()
{
    QFETCH(QList<ValueProperty>, properties);
    QFETCH(QByteArrayList, expectedKeys);

    Baloo::Result result;
    for (const auto &p : properties) {
        result.add(p.property, p.value);
    }
    result.finish();

    auto doc = result.document();
    QCOMPARE(doc.m_terms.keys(), expectedKeys);
}

void DocumentContentTest::benchmarkResultTerms_data()
{
    QTest::addColumn<int>("termCount");

    QTest::addRow("100") << 100;
    QTest::addRow("1000") << 1000;
    QTest::addRow("10000") << 10000;
}

void DocumentContentTest::benchmarkResultTerms()
{
    QFETCH(int, termCount);

    // Prepare test corpus
    // The terms have a log-log Distribution, i.e. a few terms with
    // high frequency.
    auto makeText = [](int termCount) {
        QString out;
        for (int i = 0; i < termCount; i++) {
            QString term{u" term" + QString::number(i, 16)};
            float exp = std::min(log2(termCount) - log2(i + 2.0) + 1.0, 32.0);
            int occurences = pow(1.8, exp);
            for (int j = 0; j < occurences; j++) {
                out += term;
            }
            // qDebug() << i << occurences;
        }
        return out;
    };
    QString text{makeText(termCount)};

    QBENCHMARK {
        Baloo::Result result;
        result.append(text);
        result.finish();
    }
}

QTEST_MAIN(DocumentContentTest)

#include "documentcontenttest.moc"
