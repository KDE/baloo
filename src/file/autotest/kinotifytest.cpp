/*
   Copyright (C) 2010 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kinotifytest.h"
#include "../kinotify.h"

#include <KTempDir>
#include <KRandom>
#include <KStandardDirs>
#include <qtest_kde.h>

#include <QTextStream>
#include <QFile>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include <QDir>

namespace
{
void touchFile(const QString& path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    QTextStream s(&file);
    s << KRandom::randomString(10);
}

void mkdir(const QString& path)
{
    KStandardDirs::makeDir(path);
}

void waitForSignal(QObject* object, const char* signal, int timeout = 500)
{
    QEventLoop loop;
    loop.connect(object, signal, SLOT(quit()));
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
}
}

void KInotifyTest::testDeleteFile()
{
    // create some test files
    KTempDir dir;
    const QString f1(QString::fromLatin1("%1randomJunk1").arg(dir.name()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(deleted(QString,bool)));

    // test removing a file
    QFile::remove(f1);
    waitForSignal(&kn, SIGNAL(deleted(QString,bool)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), f1);
}


void KInotifyTest::testDeleteFolder()
{
    // create some test files
    KTempDir dir;
    const QString d1(QString::fromLatin1("%1randomJunk4").arg(dir.name()));
    mkdir(d1);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(deleted(QString,bool)));

    // test removing a folder
    QDir().rmdir(d1);
    waitForSignal(&kn, SIGNAL(deleted(QString,bool)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), d1);
    // make sure we do not watch the removed folder anymore
    QVERIFY(!kn.watchingPath(d1));
}


void KInotifyTest::testCreateFolder()
{
    KTempDir dir;

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // create the subdir
    const QString d1(QString::fromLatin1("%1randomJunk1").arg(dir.name()));
    mkdir(d1);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d1);
    QVERIFY(kn.watchingPath(d1));

    // lets go one level deeper
    const QString d2 = QString::fromLatin1("%1/subdir1").arg(d1);
    mkdir(d2);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d2);
    QVERIFY(kn.watchingPath(d2));

    // although we are in the folder test lets try creating a file
    const QString f1 = QString::fromLatin1("%1/somefile1").arg(d2);
    touchFile(f1);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f1);
}


void KInotifyTest::testRenameFile()
{
    // create some test files
    KTempDir dir;
    const QString f1(QString::fromLatin1("%1randomJunk1").arg(dir.name()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QString::fromLatin1("%1randomJunk2").arg(dir.name()));
    QFile::rename(f1, f2);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f1);
    QCOMPARE(args.at(1).toString(), f2);

    // test a subsequent rename
    const QString f3(QString::fromLatin1("%1randomJunk3").arg(dir.name()));
    QFile::rename(f2, f3);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f2);
    QCOMPARE(args.at(1).toString(), f3);
}


void KInotifyTest::testMoveFile()
{
    // create some test files
    KTempDir dir1;
    KTempDir dir2;
    const QString src(QString::fromLatin1("%1randomJunk1").arg(dir1.name()));
    const QString dest(QString::fromLatin1("%1randomJunk2").arg(dir2.name()));
    touchFile(src);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir1.name(), KInotify::EventAll);
    kn.addWatch(dir2.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    QFile::rename(src, dest);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), src);
    QCOMPARE(args.at(1).toString(), dest);

    // test a subsequent move (back to the original folder)
    const QString dest2(QString::fromLatin1("%1randomJunk3").arg(dir1.name()));
    QFile::rename(dest, dest2);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), dest);
    QCOMPARE(args.at(1).toString(), dest2);
}


void KInotifyTest::testRenameFolder()
{
    // create some test files
    KTempDir dir;
    const QString f1(QString::fromLatin1("%1randomJunk1").arg(dir.name()));
    mkdir(f1);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QString::fromLatin1("%1randomJunk2").arg(dir.name()));
    QFile::rename(f1, f2);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f1);
    QCOMPARE(args.at(1).toString(), f2);

    // check the path cache
    QVERIFY(!kn.watchingPath(f1));
    QVERIFY(kn.watchingPath(f2));

    // test a subsequent rename
    const QString f3(QString::fromLatin1("%1randomJunk3").arg(dir.name()));
    QFile::rename(f2, f3);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f2);
    QCOMPARE(args.at(1).toString(), f3);

    // check the path cache
    QVERIFY(!kn.watchingPath(f1));
    QVERIFY(!kn.watchingPath(f2));
    QVERIFY(kn.watchingPath(f3));


    // KInotify claims it has updated its data structures, lets see if that is true
    // by creating a file in the new folder
    // listen to the desired signal
    const QString f4(QString::fromLatin1("%1/somefile").arg(f3));

    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // test creating a file
    touchFile(f4);

    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f4);
}


void KInotifyTest::testMoveFolder()
{
    // create some test files
    KTempDir dir1;
    KTempDir dir2;
    const QString src(QString::fromLatin1("%1randomJunk1").arg(dir1.name()));
    const QString dest(QString::fromLatin1("%1randomJunk2").arg(dir2.name()));
    mkdir(src);

    // start the inotify watcher
    KInotify kn;
    kn.addWatch(dir1.name(), KInotify::EventAll);
    kn.addWatch(dir2.name(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    QFile::rename(src, dest);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), src);
    QCOMPARE(args.at(1).toString(), dest);

    // check the path cache
    QVERIFY(!kn.watchingPath(src));
    QVERIFY(kn.watchingPath(dest));

    // test a subsequent move
    const QString dest2(QString::fromLatin1("%1randomJunk3").arg(dir1.name()));
    QFile::rename(dest, dest2);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), dest);
    QCOMPARE(args.at(1).toString(), dest2);

    // check the path cache
    QVERIFY(!kn.watchingPath(src));
    QVERIFY(!kn.watchingPath(dest));
    QVERIFY(kn.watchingPath(dest2));


    // KInotify claims it has updated its data structures, lets see if that is true
    // by creating a file in the new folder
    // listen to the desired signal
    const QString f4(QString::fromLatin1("%1/somefile").arg(dest2));

    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // test creating a file
    touchFile(f4);

    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f4);
}


void KInotifyTest::testMoveRootFolder()
{
    // create some test folders
    KTempDir dir;
    const QString src(QString::fromLatin1("%1randomJunk1").arg(dir.name()));
    const QString dest(QString::fromLatin1("%1randomJunk2").arg(dir.name()));
    mkdir(src);

    // start watching the new subfolder only
    KInotify kn;
    kn.addWatch(src, KInotify::EventAll);

    // listen for the moved signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    QFile::rename(src, dest);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QEXPECT_FAIL("", "KInotify cannot handle moving of top-level folders.", Abort);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), src);
    QCOMPARE(args.at(1).toString(), dest);

    // check the path cache
    QVERIFY(!kn.watchingPath(src));
    QVERIFY(kn.watchingPath(dest));
}

QTEST_KDEMAIN_CORE(KInotifyTest)
