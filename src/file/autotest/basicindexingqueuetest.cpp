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

#include "basicindexingqueuetest.h"
#include "fileindexerconfigutils.h"

#include "../fileindexerconfig.h"
#include "../basicindexingqueue.h"
#include "../database.h"

#include "qtest_kde.h"

#include <QSignalSpy>
#include <QEventLoop>

using namespace Baloo;

void BasicIndexingQueueTest::testSimpleDirectoryStructure()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");
    dirs << QLatin1String("home/2");
    dirs << QLatin1String("home/kde/");
    dirs << QLatin1String("home/kde/1");
    dirs << QLatin1String("home/docs/");
    dirs << QLatin1String("home/docs/1");

    QScopedPointer<KTempDir> dir(Test::createTmpFilesAndFolders(dirs));
    QString p = dir->name();

    QStringList includeFolders;
    includeFolders << dir->name() + QLatin1String("home");

    QStringList excludeFolders;
    excludeFolders << dir->name() + QLatin1String("home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    KTempDir dbDir;
    Database db;
    db.setPath(dbDir.name());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);
    queue.enqueue(FileMapping(p + QLatin1String("home")));

    QSignalSpy spy(&queue, SIGNAL(newDocument(uint,Xapian::Document)));
    queue.resume();

    QEventLoop loop;
    connect(&queue, SIGNAL(finishedIndexing()), &loop, SLOT(quit()));
    loop.exec();

    // kde and kde/1 are not indexed
    QCOMPARE(spy.count(), 5);
}

QTEST_KDEMAIN_CORE(BasicIndexingQueueTest)
