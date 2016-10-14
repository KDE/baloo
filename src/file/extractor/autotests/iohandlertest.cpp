/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QTest>
#include <QTemporaryFile>
#include <QFile>
#include <QVector>
#include <QByteArray>
#include <QDebug>
#include <cmath>

#include "../iohandler.h"

namespace Baloo {
class IOHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInput();
};
}

using namespace Baloo;

void IOHandlerTest::testInput()
{
    QTemporaryFile stdIn;
    stdIn.open();
    IOHandler io(stdIn.handle());

    QVector<quint64> ids;
    quint32 size = 10;
    ids.reserve(size);
    QByteArray ba;
    ba.append(reinterpret_cast<char*>(&size), sizeof(quint32));

    for (quint64 i = 0; i < 10; ++i) {
        quint64 a = std::pow(16, i);
        ids.append(a);
        ba.append(reinterpret_cast<char*>(&a), sizeof(quint64));
    }

    stdIn.write(ba.data(), ba.size());
    //we will de dealing with stdin actually which is a sequential device
    //hence io handler expects to read from start.
    stdIn.reset();
    io.newBatch();

    quint32 i = 0;

    while (!io.atEnd()) {
        QCOMPARE(io.nextId(), ids.at(i++));
    }

    QCOMPARE(i, size);
}

QTEST_MAIN(IOHandlerTest)

#include "iohandlertest.moc"
