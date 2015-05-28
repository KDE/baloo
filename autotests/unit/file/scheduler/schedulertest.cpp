/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "schedulertest.h"
#include "../fileindexerconfigutils.h"
#include "../../indexscheduler.h"
#include "../../database.h"
#include "../../fileindexerconfig.h"
#include "../../basicindexingqueue.h"
#include "../../fileindexingqueue.h"
#include "../../eventmonitor.h"
#include "xapiandatabase.h"

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>

#include <Solid/PowerManagement>

using namespace Baloo;

SchedulerTest::IndexingData SchedulerTest::fetchIndexingData(Database& db)
{
    Xapian::Database* xdb = db.xapianDatabase()->db();
    xdb->reopen();
    Xapian::Enquire enquire(*xdb);

    IndexingData data;

    enquire.set_query(Xapian::Query("Z1"));
    Xapian::MSet mset = enquire.get_mset(0, 10000000);
    data.phaseOne = mset.size();

    enquire.set_query(Xapian::Query("Z2"));
    mset = enquire.get_mset(0, 10000000);
    data.phaseTwo = mset.size();

    enquire.set_query(Xapian::Query("Z-1"));
    mset = enquire.get_mset(0, 10000000);
    data.failed = mset.size();

    return data;
}

void SchedulerTest::test()
{
    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    uint numFiles = 20;
    QTemporaryDir fileDir;
    // The -1 is because the folder which contains all these files will also
    // be indexed, and will count as a file
    for (uint i = 0; i < numFiles-1; i++) {
        QFile file(fileDir.path() + QDir::separator() + QString::number(i) + QLatin1String(".txt"));
        file.open(QIODevice::WriteOnly);

        QTextStream stream(&file);
        stream << i;
    }

    Test::writeIndexerConfig(QStringList() << fileDir.path(), QStringList());

    FileIndexerConfig config;
    IndexScheduler scheduler(&db, &config);
    scheduler.m_fileIQ->setBatchSize(2);
    scheduler.m_fileIQ->setMaxSize(10);
    scheduler.m_fileIQ->setTestMode(true);
    scheduler.m_eventMonitor->disconnect(&scheduler);
    scheduler.m_eventMonitor->m_isIdle = false;
    scheduler.m_eventMonitor->m_isOnBattery = false;

    QSignalSpy spy1(&scheduler, SIGNAL(indexingStarted()));
    QSignalSpy spy2(&scheduler, SIGNAL(basicIndexingDone()));
    QSignalSpy spy3(&scheduler, SIGNAL(fileIndexingDone()));

    QVERIFY(!scheduler.isIndexing());
    QVERIFY(!scheduler.isSuspended());
    QVERIFY(scheduler.m_basicIQ->isEmpty());
    QVERIFY(scheduler.m_fileIQ->isEmpty());
    QVERIFY(scheduler.m_commitQ->isEmpty());
    QCOMPARE(scheduler.userStatusString(), QString("File indexer is idle."));

    QStringList statuses;
    connect(&scheduler, &IndexScheduler::statusStringChanged, [&]() { statuses << scheduler.userStatusString(); });

    scheduler.updateAll();
    QEventLoop loop;
    connect(&scheduler, &IndexScheduler::indexingStopped, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);

    QVERIFY(!scheduler.isIndexing());
    QVERIFY(!scheduler.isSuspended());
    QVERIFY(scheduler.m_basicIQ->isEmpty());
    QVERIFY(scheduler.m_fileIQ->isEmpty());
    QVERIFY(scheduler.m_commitQ->isEmpty());
    QCOMPARE(scheduler.userStatusString(), QString("File indexer is idle."));

    QCOMPARE(statuses.size(), 3);
    QVERIFY(statuses[0].contains("recent changes", Qt::CaseInsensitive));
    QVERIFY(statuses[1].contains("indexing", Qt::CaseInsensitive));
    QVERIFY(statuses[2].contains("idle", Qt::CaseInsensitive));

    Xapian::Database* xdb = db.xapianDatabase()->db();
    xdb->reopen();

    QCOMPARE(xdb->get_doccount(), numFiles);

    IndexingData data = fetchIndexingData(db);
    QCOMPARE(data.phaseOne, 0);
    QCOMPARE(data.phaseTwo, (int)numFiles);
    QCOMPARE(data.failed, 0);
}

void SchedulerTest::testBatterySuspend()
{
    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    uint numFiles = 10;
    QTemporaryDir fileDir;
    // The -1 is because the folder which contains all these files will also
    // be indexed, and will count as a file
    for (uint i = 0; i < numFiles-1; i++) {
        QFile file(fileDir.path() + QDir::separator() + QString::number(i) + QLatin1String(".txt"));
        file.open(QIODevice::WriteOnly);

        QTextStream stream(&file);
        stream << i;
    }

    Test::writeIndexerConfig(QStringList() << fileDir.path(), QStringList());

    FileIndexerConfig config;
    IndexScheduler scheduler(&db, &config);
    scheduler.m_fileIQ->setTestMode(true);
    scheduler.m_eventMonitor->m_isIdle = false;
    scheduler.m_eventMonitor->m_isOnBattery = false;
    Solid::PowerManagement::notifier()->disconnect(scheduler.m_eventMonitor);

    scheduler.updateAll();
    QEventLoop loop;
    connect(&scheduler, &IndexScheduler::basicIndexingDone, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(&scheduler, &IndexScheduler::basicIndexingDone, &loop, &QEventLoop::quit);

    QVERIFY(scheduler.m_basicIQ->isEmpty());

    IndexingData data = fetchIndexingData(db);
    QCOMPARE(data.phaseOne, 0);
    QCOMPARE(data.phaseTwo, 0);
    QCOMPARE(data.failed, 0);

    // The changes have still not been committed, so the fileIQ will be empty
    QVERIFY(scheduler.m_fileIQ->isEmpty());
    QVERIFY(!scheduler.m_fileIQ->isSuspended());

    scheduler.m_commitQ->commit();
    QVERIFY(!scheduler.m_fileIQ->isEmpty());
    QVERIFY(!scheduler.m_fileIQ->isSuspended());
    QCOMPARE(scheduler.m_fileIQ->delay(), 500);

    data = fetchIndexingData(db);
    // Folders automatically go to phase2
    QCOMPARE(data.phaseOne, (int)numFiles-1);
    QCOMPARE(data.phaseTwo, 1);
    QCOMPARE(data.failed, 0);

    // Conserve resources
    scheduler.m_eventMonitor->slotPowerManagementStatusChanged(true);

    QVERIFY(!scheduler.m_fileIQ->isEmpty());
    QVERIFY(scheduler.m_fileIQ->isSuspended());

    scheduler.m_eventMonitor->slotPowerManagementStatusChanged(false);
    QVERIFY(!scheduler.m_fileIQ->isSuspended());

    QSignalSpy spy1(&scheduler, SIGNAL(indexingStarted()));
    QSignalSpy spy2(&scheduler, SIGNAL(basicIndexingDone()));
    QSignalSpy spy3(&scheduler, SIGNAL(fileIndexingDone()));

    connect(&scheduler, &IndexScheduler::fileIndexingDone, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy1.size(), 0);
    QCOMPARE(spy2.size(), 0);
    QCOMPARE(spy3.size(), 1);

    data = fetchIndexingData(db);
    QCOMPARE(data.phaseOne, 0);
    QCOMPARE(data.phaseTwo, (int)numFiles);
    QCOMPARE(data.failed, 0);
}

void SchedulerTest::testIdle()
{
    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    uint numFiles = 10;
    QTemporaryDir fileDir;
    for (uint i = 0; i < numFiles-1; i++) {
        QFile file(fileDir.path() + QDir::separator() + QString::number(i) + QLatin1String(".txt"));
        file.open(QIODevice::WriteOnly);

        QTextStream stream(&file);
        stream << i;
    }

    Test::writeIndexerConfig(QStringList() << fileDir.path(), QStringList());

    FileIndexerConfig config;
    IndexScheduler scheduler(&db, &config);
    scheduler.m_fileIQ->setTestMode(true);
    scheduler.m_eventMonitor->m_isIdle = false;
    scheduler.m_eventMonitor->m_isOnBattery = false;
    Solid::PowerManagement::notifier()->disconnect(scheduler.m_eventMonitor);

    scheduler.updateAll();
    QEventLoop loop;
    connect(&scheduler, &IndexScheduler::basicIndexingDone, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(&scheduler, &IndexScheduler::basicIndexingDone, &loop, &QEventLoop::quit);

    QVERIFY(scheduler.m_basicIQ->isEmpty());

    QVERIFY(!scheduler.m_fileIQ->isSuspended());
    QCOMPARE(scheduler.m_fileIQ->delay(), 500);

    scheduler.m_eventMonitor->slotIdleTimeoutReached();

    QVERIFY(!scheduler.m_fileIQ->isSuspended());
    QCOMPARE(scheduler.m_fileIQ->delay(), 0);

    scheduler.m_eventMonitor->slotResumeFromIdle();

    QVERIFY(!scheduler.m_fileIQ->isSuspended());
    QCOMPARE(scheduler.m_fileIQ->delay(), 500);
}

QTEST_MAIN(SchedulerTest)
