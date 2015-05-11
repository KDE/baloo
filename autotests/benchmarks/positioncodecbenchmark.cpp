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

#include "positioncodec.h"
#include "positioninfo.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class PositionCodecBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void PositionCodecBenchmark::test()
{
    PositionCodec codec;

    QVector<PositionInfo> vec;
    for (int i = 0; i < 5000; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        vec << info;
    }

    QBENCHMARK {
        QByteArray data = codec.encode(vec);
        codec.decode(data);
    }
}

QTEST_MAIN(PositionCodecBenchmark)

#include "positioncodecbenchmark.moc"
