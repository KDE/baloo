/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingcodec.h"

#include <QObject>
#include <QTest>

using namespace Baloo;

class PostingCodecTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        PostingCodec codec;

        QVector<quint64> vec = {1, 2, 9, 12};
        QByteArray arr = codec.encode(vec);
        QVERIFY(!arr.isEmpty());

        QVector<quint64> vec2 = codec.decode(arr);
        QCOMPARE(vec2, vec);
    }

};

QTEST_MAIN(PostingCodecTest)

#include "postingcodectest.moc"
