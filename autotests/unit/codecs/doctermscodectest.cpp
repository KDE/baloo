/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "doctermscodec.h"

#include <QObject>
#include <QTest>

using namespace Baloo;

class DocTermsCodecTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        DocTermsCodec codec;

        QVector<QByteArray> vec = {"ab", "abc", "dar", "darwin"};
        QByteArray arr = codec.encode(vec);
        QVERIFY(!arr.isEmpty());

        QVector<QByteArray> vec2 = codec.decode(arr);
        QCOMPARE(vec2, vec);
    }
};

QTEST_MAIN(DocTermsCodecTest)

#include "doctermscodectest.moc"
