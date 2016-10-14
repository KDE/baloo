/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2016 Christian Ehrlicher <ch.ehrlicher@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCryptographicHash>
#include <QtTest>
#include "positioncodec.h"
#include "positioninfo.h"

using namespace Baloo;

class PositionCodecTest : public QObject
{
Q_OBJECT
public:
    PositionCodecTest() = default;
    ~PositionCodecTest() = default;
private Q_SLOTS:
    void initTestCase();
    void checkEncodeOutput();
private:
    QVector<PositionInfo> m_data;
};

QTEST_MAIN ( PositionCodecTest )

void PositionCodecTest::initTestCase()
{
    m_data.clear();
    m_data.reserve(100);
    for(int i = 0; i < 100; ++i)
    {
        PositionInfo info;
        info.docId = (i + 1) * 4711;
        info.positions.reserve(3000);
        for (int j = 0; j < 3000; j++)
            info.positions.append(((j + 1) * 42) / info.docId);
        m_data.append(info);
    }
}

void PositionCodecTest::checkEncodeOutput()
{
    PositionCodec pc;
    const QByteArray ba = pc.encode(m_data);
    QCOMPARE(ba.size(), 301000);
    const QByteArray md5 = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
    QCOMPARE(md5, QByteArray("d44a606d301937bef105411c0ee77a88"));
    // and now decode the whole stuff
    QVector<PositionInfo> decodedData = pc.decode(ba);
    QCOMPARE(m_data, decodedData);
}

#include "positioncodectest.moc"
