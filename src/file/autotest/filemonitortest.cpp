/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "filemonitortest.h"
#include "filemonitor.h"

#include <QTest>
#include <QSignalSpy>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Baloo;

void FileMonitorTest::test()
{
    QString file("/tmp/t");
    FileMonitor monitor;
    monitor.addFile(file);

    QSignalSpy spy(&monitor, SIGNAL(fileMetaDataChanged(QString)));

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QList<QString> list;
    list << file;

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(list);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);
    QTest::qWait(20);

    QCOMPARE(spy.count(), 1);

    QList<QVariant> variantList = spy.at(0);
    QCOMPARE(variantList.size(), 1);

    QVariant var = variantList.front();
    QCOMPARE(var.type(), QVariant::String);
    QCOMPARE(var.toString(), file);
}

QTEST_MAIN(FileMonitorTest)
