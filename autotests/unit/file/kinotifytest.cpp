/*
   Copyright (C) 2010 by Sebastian Trueg <trueg at kde.org>
   Copyright (C) 2014 by Vishesh Handa <vhanda@kde.org>

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

#include "kinotify.h"

#include <QTemporaryDir>
#include <KRandom>

#include <QTextStream>
#include <QFile>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include <QDir>
#include <QTest>

class KInotifyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDeleteFile();
    void testDeleteFolder();
    void testCreateFolder();
    void testRenameFile();
    void testMoveFile();
    void testRenameFolder();
    void testMoveFolder();
    void testMoveRootFolder();
    void testFileClosedAfterWrite();
};

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
    QDir().mkpath(path);
    QVERIFY(QDir(path).exists());
}

void waitForSignal(QObject* object, const char* signal, int timeout = 500)
{
    QEventLoop loop;
    loop.connect(object, signal, SLOT(quit()));
    QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
    loop.exec();
}
}

void KInotifyTest::testDeleteFile()
{
    // create some test files
    QTemporaryDir dir;
    const QString f1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

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
    QTemporaryDir dir;
    const QString d1(QStringLiteral("%1/randomJunk4").arg(dir.path()));
    mkdir(d1);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(deleted(QString,bool)));

    // test removing a folder
    QDir().rmdir(d1);
    waitForSignal(&kn, SIGNAL(deleted(QString,bool)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toString(), d1);
    QCOMPARE(spy.first().at(1).toBool(), true);
    // make sure we do not watch the removed folder anymore
    QVERIFY(!kn.watchingPath(d1));
}


void KInotifyTest::testCreateFolder()
{
    QTemporaryDir dir;

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // create the subdir
    const QString d1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    mkdir(d1);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d1);
    QVERIFY(kn.watchingPath(d1));

    // lets go one level deeper
    const QString d2 = QStringLiteral("%1/subdir1").arg(d1);
    mkdir(d2);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d2);
    QVERIFY(kn.watchingPath(d2));

    // although we are in the folder test lets try creating a file
    const QString f1 = QStringLiteral("%1/somefile1").arg(d2);
    touchFile(f1);
    waitForSignal(&kn, SIGNAL(created(QString,bool)));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f1);
}


void KInotifyTest::testRenameFile()
{
    // create some test files
    QTemporaryDir dir;
    const QString f1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    QFile::rename(f1, f2);

    // check the desired signal
    waitForSignal(&kn, SIGNAL(moved(QString,QString)));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f1);
    QCOMPARE(args.at(1).toString(), f2);

    // test a subsequent rename
    const QString f3(QStringLiteral("%1/randomJunk3").arg(dir.path()));
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
    QTemporaryDir dir1;
    QTemporaryDir dir2;
    const QString src(QStringLiteral("%1/randomJunk1").arg(dir1.path()));
    const QString dest(QStringLiteral("%1/randomJunk2").arg(dir2.path()));
    touchFile(src);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir1.path(), KInotify::EventAll);
    kn.addWatch(dir2.path(), KInotify::EventAll);

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
    const QString dest2(QStringLiteral("%1/randomJunk3").arg(dir1.path()));
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
    QTemporaryDir dir;
    const QString f1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    mkdir(f1);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QStringLiteral("%1/randomJunk2").arg(dir.path()));
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
    const QString f3(QStringLiteral("%1/randomJunk3").arg(dir.path()));
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
    const QString f4(QStringLiteral("%1/somefile").arg(f3));

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
    QTemporaryDir dir1;
    QTemporaryDir dir2;
    const QString src(QStringLiteral("%1/randomJunk1").arg(dir1.path()));
    const QString dest(QStringLiteral("%1/randomJunk2").arg(dir2.path()));
    mkdir(src);

    // start the inotify watcher
    KInotify kn(0 /*no config*/);
    kn.addWatch(dir1.path(), KInotify::EventAll);
    kn.addWatch(dir2.path(), KInotify::EventAll);

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
    const QString dest2(QStringLiteral("%1/randomJunk3").arg(dir1.path()));
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
    const QString f4(QStringLiteral("%1/somefile").arg(dest2));

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
    QTemporaryDir dir;
    const QString src(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    const QString dest(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    mkdir(src);

    // start watching the new subfolder only
    KInotify kn(0 /*no config*/);
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

void KInotifyTest::testFileClosedAfterWrite()
{
    QTemporaryDir dir;
    touchFile(dir.path() + QLatin1String("/file"));

    KInotify kn(0 /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    QSignalSpy spy(&kn, SIGNAL(closedWrite(QString)));
    touchFile(dir.path() + QLatin1String("/file"));

    waitForSignal(&kn, SIGNAL(closedWrite(QString)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).first().toString(), QString(dir.path() + QLatin1String("/file")));
}


QTEST_GUILESS_MAIN(KInotifyTest)

#include "kinotifytest.moc"
