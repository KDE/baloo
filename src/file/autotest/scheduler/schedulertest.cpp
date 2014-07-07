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
#include <commitqueue.h>
#include "xapiandatabase.h"

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>

using namespace Baloo;

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

    QSignalSpy spy1(&scheduler, SIGNAL(indexingStarted()));
    QSignalSpy spy2(&scheduler, SIGNAL(basicIndexingDone()));
    QSignalSpy spy3(&scheduler, SIGNAL(fileIndexingDone()));

    scheduler.updateAll();
    QEventLoop loop;
    connect(&scheduler, SIGNAL(indexingStopped()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);

    Xapian::Database* xdb = db.xapianDatabase()->db();
    xdb->reopen();

    QCOMPARE(xdb->get_doccount(), numFiles);

    Xapian::Enquire enquire(*xdb);

    enquire.set_query(Xapian::Query("Z1"));
    Xapian::MSet mset = enquire.get_mset(0, 10000000);
    int phaseOne = mset.size();

    enquire.set_query(Xapian::Query("Z2"));
    mset = enquire.get_mset(0, 10000000);
    uint phaseTwo = mset.size();

    enquire.set_query(Xapian::Query("Z-1"));
    mset = enquire.get_mset(0, 10000000);
    int failed = mset.size();

    QCOMPARE(phaseOne, 0);
    QCOMPARE(phaseTwo, numFiles);
    QCOMPARE(failed, 0);
}

QTEST_MAIN(SchedulerTest)
