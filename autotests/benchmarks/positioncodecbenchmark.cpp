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
    void initTestCase();
    // data 1 - a lot of positions, small amount of PositionInfo
    void benchEncodeData1();
    void benchDecodeData1();
    // data 2 - few positions, large amount of PositionInfo
    void benchEncodeData2();
    void benchDecodeData2();
private:
    QVector<PositionInfo> m_benchmarkData1;
    QVector<PositionInfo> m_benchmarkData2;
};

void PositionCodecBenchmark::initTestCase()
{
    m_benchmarkData1.clear();
    m_benchmarkData1.reserve(100);
    for(int i = 0; i < 100; ++i)
    {
        PositionInfo info;
        info.docId = (i + 1) * 4711;
        info.positions.reserve(3000);
        for (int j = 0; j < 3000; j++)
            info.positions.append(((j + 1) * 42) / info.docId);
        m_benchmarkData1.append(info);
    }

    m_benchmarkData2.clear();
    m_benchmarkData2.reserve(5000);
    for (int i = 0; i < 5000; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        m_benchmarkData2.append(info);
    }

}

void PositionCodecBenchmark::benchEncodeData1()
{
    PositionCodec pc;
    QBENCHMARK { pc.encode(m_benchmarkData1); }
}

void PositionCodecBenchmark::benchDecodeData1()
{
    PositionCodec pc;
    const QByteArray ba = pc.encode(m_benchmarkData1);
    QBENCHMARK { pc.decode(ba); }
}

void PositionCodecBenchmark::benchEncodeData2()
{
    PositionCodec pc;
    QBENCHMARK { pc.encode(m_benchmarkData2); }
}

void PositionCodecBenchmark::benchDecodeData2()
{
    PositionCodec pc;
    const QByteArray ba = pc.encode(m_benchmarkData2);
    QBENCHMARK { pc.decode(ba); }
}

QTEST_MAIN(PositionCodecBenchmark)

#include "positioncodecbenchmark.moc"
