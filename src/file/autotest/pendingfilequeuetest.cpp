/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

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

#include "pendingfilequeuetest.h"
#include "../pendingfilequeue.h"

#include <qtest.h>
#include <qsignalspy.h>
#include <qtimer.h>

using namespace Baloo;

namespace
{
void loopWait(int msecs)
{
    QEventLoop loop;
    QTimer::singleShot(msecs, &loop, SLOT(quit()));
    loop.exec();
}
}

PendingFileQueueTest::PendingFileQueueTest()
{
}

void PendingFileQueueTest::testTimeout()
{
    QString myUrl(QLatin1String("/tmp"));

    // enqueue one url and then make sure it is not emitted before the timeout
    PendingFileQueue queue;
    queue.setTimeout(3);
    queue.setWaitTimeout(2);

    QSignalSpy spy(&queue, SIGNAL(indexFile(QString)));
    PendingFile file(myUrl);
    file.setModified();
    queue.enqueueUrl(file);

    // The signal should be emitted immediately
    QTest::qWait(20);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().first().toString(), myUrl);

    // wait for 1 seconds
    loopWait(1000);

    queue.enqueueUrl(file);
    // the signal should not have been emitted yet
    QVERIFY(spy.isEmpty());

    // wait for 1 seconds
    loopWait(1000);
    QVERIFY(spy.isEmpty());

    // wait another 2 seconds
    loopWait(2000);

    // now the signal should have been emitted
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().first().toString(), myUrl);
}

void PendingFileQueueTest::testRequeue()
{
    QString myUrl(QLatin1String("/tmp"));

    // enqueue one url and then make sure it is not emitted before the timeout
    PendingFileQueue queue;
    queue.setTimeout(3);
    queue.setWaitTimeout(2);

    QSignalSpy spy(&queue, SIGNAL(indexFile(QString)));
    PendingFile file(myUrl);
    file.setModified();
    queue.enqueueUrl(file);

    // The signal should be emitted immediately
    QTest::qWait(20);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().first().toString(), myUrl);
    QVERIFY(spy.isEmpty());

    queue.enqueueUrl(file);
    // wait for 1 seconds
    loopWait(1000);

    // the signal should not have been emitted yet
    QVERIFY(spy.isEmpty());

    // re-queue the url
    queue.enqueueUrl(file);

    // wait another 1 seconds
    loopWait(1000);

    // the signal should not have been emitted yet, because after re-queing it
    // it should wait a total of 3 seconds before emitting it
    // 3 seconds = timeout value
    QVERIFY(spy.isEmpty());

    // wait another 3 seconds
    loopWait(3000);

    // now the signal should have been emitted
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().first().toString(), myUrl);
}

QTEST_MAIN(PendingFileQueueTest)
