/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingcodec.h"

#include <QObject>
#include <QRegularExpression>
#include <QTest>

using namespace Baloo;

class PostingCodecTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        QVector<quint64> vec = {1, 2, 9, 12};
        QByteArray arr = PostingCodec::encode(vec);
        QVERIFY(!arr.isEmpty());

        QVector<quint64> vec2 = PostingCodec::decode(arr);
        QCOMPARE(vec2, vec);
    }

    void testDecodeRejectsAPartialEntry()
    {
        // A length that is not a multiple of sizeof(quint64) means the value
        // is damaged, so none of it is trusted.
        const QVector<quint64> vec = {1, 2, 9, 12};
        QByteArray arr = PostingCodec::encode(vec);
        arr.chop(3); // three whole entries plus five trailing bytes
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("not a multiple")));
        QCOMPARE(PostingCodec::decode(arr), QVector<quint64>{});

        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("not a multiple")));
        QCOMPARE(PostingCodec::decode(QByteArray(5, 'x')), QVector<quint64>{});
    }
};

QTEST_MAIN(PostingCodecTest)

#include "postingcodectest.moc"
