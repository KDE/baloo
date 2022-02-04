/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg at kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kinotify.h"

#include <QTemporaryDir>
#include <KRandom>

#include <QTextStream>
#include <QFile>
#include <QSignalSpy>
#include <QDir>
#include <QTest>

#include <stdio.h>

class KInotifyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDeleteFile();
    void testDeleteFolder();
    void testCreateFolder();
    void testRenameFile();
    void testRenameDeleteFile();
    void testMoveFile();
    void testRenameFolder();
    void testMoveFolder();
    void testMoveFromUnwatchedFolder();
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
}

void KInotifyTest::testDeleteFile()
{
    // create some test files
    QTemporaryDir dir;
    const QString f1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(deleted(QString,bool)));

    // test removing a file
    QFile::remove(f1);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), f1);
}


void KInotifyTest::testDeleteFolder()
{
    // create some test files
    QTemporaryDir dir;
    const QString d1(QStringLiteral("%1/randomJunk4/").arg(dir.path()));
    mkdir(d1);

    // start the inotify watcher
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(deleted(QString,bool)));

    // test removing a folder
    QDir().rmdir(d1);
    QVERIFY(spy.wait());
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
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // create the subdir
    const QString d1(QStringLiteral("%1/randomJunk1/").arg(dir.path()));
    mkdir(d1);
    QVERIFY(createdSpy.wait());
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d1);
    QVERIFY(kn.watchingPath(d1));

    // lets go one level deeper
    const QString d2 = QStringLiteral("%1subdir1/").arg(d1);
    mkdir(d2);
    QVERIFY(createdSpy.wait());
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), d2);
    QVERIFY(kn.watchingPath(d2));

    // although we are in the folder test lets try creating a file
    const QString f1 = QStringLiteral("%1somefile1").arg(d2);
    touchFile(f1);
    QVERIFY(createdSpy.wait());
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
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    rename(f1.toLocal8Bit().constData(), f2.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f1);
    QCOMPARE(args.at(1).toString(), f2);

    // test a subsequent rename
    const QString f3(QStringLiteral("%1/randomJunk3").arg(dir.path()));
    rename(f2.toLocal8Bit().constData(), f3.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f2);
    QCOMPARE(args.at(1).toString(), f3);
}

void KInotifyTest::testRenameDeleteFile()
{
    // create some test files
    QTemporaryDir dir;
    const QString f1(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    touchFile(f1);

    // start the inotify watcher
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    const QString f2(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    rename(f1.toLocal8Bit().constData(), f2.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), f1);
    QCOMPARE(args.at(1).toString(), f2);

    // test a subsequent delete
    QSignalSpy spy1(&kn, SIGNAL(deleted(QString,bool)));
    QFile::remove(f2);

    // check the desired signal
    QVERIFY(spy1.wait());
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy1.takeFirst().at(0).toString(), f2);

    QVERIFY(spy.isEmpty());
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
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir1.path(), KInotify::EventAll);
    kn.addWatch(dir2.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    rename(src.toLocal8Bit().constData(), dest.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), src);
    QCOMPARE(args.at(1).toString(), dest);

    // test a subsequent move (back to the original folder)
    const QString dest2(QStringLiteral("%1/randomJunk3").arg(dir1.path()));
    rename(dest.toLocal8Bit().constData(), dest2.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), dest);
    QCOMPARE(args.at(1).toString(), dest2);
}

void KInotifyTest::testRenameFolder()
{
    // create some test files
    QTemporaryDir dir;
    const QString d1(QStringLiteral("%1/randomJunk1/").arg(dir.path()));
    mkdir(d1);

    // start the inotify watcher
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually rename the folder
    const QString d2(QStringLiteral("%1/randomJunk2/").arg(dir.path()));
    rename(d1.toLocal8Bit().constData(), d2.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), d1);
    QCOMPARE(args.at(1).toString(), d2);

    // check the path cache
    QVERIFY(!kn.watchingPath(d1));
    QVERIFY(kn.watchingPath(d2));

    // test a subsequent rename
    const QString d3(QStringLiteral("%1/randomJunk3/").arg(dir.path()));
    rename(d2.toLocal8Bit().constData(), d3.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), d2);
    QCOMPARE(args.at(1).toString(), d3);

    // check the path cache
    QVERIFY(!kn.watchingPath(d1));
    QVERIFY(!kn.watchingPath(d2));
    QVERIFY(kn.watchingPath(d3));


    // KInotify claims it has updated its data structures, lets see if that is true
    // by creating a file in the new folder
    // listen to the desired signal
    const QString f4(QStringLiteral("%1somefile").arg(d3));

    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // test creating a file
    touchFile(f4);

    QVERIFY(createdSpy.wait());
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f4);
}

void KInotifyTest::testMoveFolder()
{
    // create some test files
    QTemporaryDir dir1;
    QTemporaryDir dir2;
    const QString src(QStringLiteral("%1/randomJunk1/").arg(dir1.path()));
    const QString dest(QStringLiteral("%1/randomJunk2/").arg(dir2.path()));
    mkdir(src);

    // start the inotify watcher
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir1.path(), KInotify::EventAll);
    kn.addWatch(dir2.path(), KInotify::EventAll);

    // listen to the desired signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    rename(src.toLocal8Bit().constData(), dest.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), src);
    QCOMPARE(args.at(1).toString(), dest);

    // check the path cache
    QVERIFY(!kn.watchingPath(src));
    QVERIFY(kn.watchingPath(dest));

    // test a subsequent move
    const QString dest2(QStringLiteral("%1/randomJunk3/").arg(dir1.path()));
    rename(dest.toLocal8Bit().constData(), dest2.toLocal8Bit().constData());

    // check the desired signal
    QVERIFY(spy.wait());
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
    const QString f4(QStringLiteral("%1somefile").arg(dest2));

    QSignalSpy createdSpy(&kn, SIGNAL(created(QString,bool)));

    // test creating a file
    touchFile(f4);

    QVERIFY(createdSpy.wait());
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.takeFirst().at(0).toString(), f4);
}

void KInotifyTest::testMoveFromUnwatchedFolder()
{
    // create some test folders
    QTemporaryDir dir;
    const QString src(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    const QString dest(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    mkdir(src);
    mkdir(dest);

    // Start watching only for destination
    KInotify kn(nullptr);
    kn.addWatch(dest, KInotify::EventAll);
    QSignalSpy spy(&kn, &KInotify::created);

    // Create stuff inside src
    mkdir(QStringLiteral("%1/sub").arg(src));
    touchFile(QStringLiteral("%1/sub/file1").arg(src));
    mkdir(QStringLiteral("%1/sub/sub1").arg(src));
    touchFile(QStringLiteral("%1/sub/sub1/file2").arg(src));

    // Now move
    QFile::rename(QStringLiteral("%1/sub").arg(src),
            QStringLiteral("%1/sub").arg(dest));

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 4);

    // Checking if watches are installed
    QSignalSpy spy1(&kn, &KInotify::deleted);
    QDir dstdir(QStringLiteral("%1/sub").arg(dest));
    dstdir.removeRecursively();

    QVERIFY(spy1.wait());
    QCOMPARE(spy1.count(), 4);
}

void KInotifyTest::testMoveRootFolder()
{
    // create some test folders
    QTemporaryDir dir;
    const QString src(QStringLiteral("%1/randomJunk1").arg(dir.path()));
    const QString dest(QStringLiteral("%1/randomJunk2").arg(dir.path()));
    mkdir(src);

    // start watching the new subfolder only
    KInotify kn(nullptr /*no config*/);
    kn.addWatch(src, KInotify::EventAll);

    // listen for the moved signal
    QSignalSpy spy(&kn, SIGNAL(moved(QString,QString)));

    // actually move the file
    QFile::rename(src, dest);

    // check the desired signal
    QEXPECT_FAIL("", "KInotify cannot handle moving of top-level folders.", Abort);
    QVERIFY(spy.wait(500));
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

    KInotify kn(nullptr /*no config*/);
    kn.addWatch(dir.path(), KInotify::EventAll);

    QSignalSpy spy(&kn, SIGNAL(closedWrite(QString)));
    touchFile(dir.path() + QLatin1String("/file"));

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).first().toString(), QString(dir.path() + QLatin1String("/file")));
}


QTEST_GUILESS_MAIN(KInotifyTest)

#include "kinotifytest.moc"
