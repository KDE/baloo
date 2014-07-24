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

void FileMonitorTest::init()
{
   // monitor =  FileMonitor();
}

void FileMonitorTest::cleanup()
{
    //monitor.de
}

void FileMonitorTest::test()
{
    FileMonitor monitor;

    QString file(QLatin1String("/tmp/t"));
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

    QEventLoop loop;
    connect(&monitor, SIGNAL(fileMetaDataChanged(QString)),
            &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(spy.count(), 1);

    QList<QVariant> variantList = spy.at(0);
    QCOMPARE(variantList.size(), 1);

    QVariant var = variantList.front();
    QCOMPARE(var.type(), QVariant::String);
    QCOMPARE(var.toString(), file);
}

void FileMonitorTest::testAddFileShouldReturnOneFileIfOneFileAdded()
{
    FileMonitor monitor;

    QString filePath = getValidFilePath();
    monitor.addFile(filePath);

    QStringList actualList= monitor.files();

    QCOMPARE(actualList.count(),1);
    QCOMPARE(actualList.first(),filePath);
}

void FileMonitorTest::testAddFileShouldReturnTwoFilesIfTwoFilesAdded()
{
    FileMonitor monitor;

    QString filePath1 = getValidFilePath();
    QString filePath2 = getValidFilePath();
    monitor.addFile(filePath1);
    monitor.addFile(filePath2);

    QStringList actualList= monitor.files();

    QCOMPARE(actualList.count(),2);
    QVERIFY(actualList.contains(filePath1));
    QVERIFY(actualList.contains(filePath2));
}

void FileMonitorTest::testAddFileShouldRemoveTailingSlash()
{
    FileMonitor monitor;

    QString filePath1(QLatin1String("/tmp/t/"));
    QString expectedFilePath(QLatin1String("/tmp/t"));
    monitor.addFile(filePath1);

    QStringList actualList= monitor.files();
    QCOMPARE(actualList.first(),expectedFilePath);
}

void FileMonitorTest::testAddFileShouldNotAddNotLocalUrl()
{
     FileMonitor monitor;
     QUrl fileUrl(QLatin1String("http://sdf.df"));

     monitor.addFile(fileUrl);
     QStringList actualList= monitor.files();

     QCOMPARE(actualList.count(),0);
}

void FileMonitorTest::testAddFileShouldAddLocalUrl()
{
     FileMonitor monitor;
     QUrl fileUrl(QLatin1String("/tmp/t"));

     monitor.addFile(fileUrl);
     QStringList actualList= monitor.files();

     QCOMPARE(actualList.count(),0);
}
void FileMonitorTest::testClearIfClearAfterOneFileAddedFilesShouldReturn0Items()
{
    FileMonitor monitor;
    QUrl fileUrl(QLatin1String("/tmp/t"));

    monitor.addFile(fileUrl);

    QStringList actualList= monitor.files();

    QCOMPARE(actualList.count(),0);
}
void FileMonitorTest::testSetFilesIfSetFilesWithOneElementFilesShouldReturn1Item()
{
    FileMonitor monitor;

    QStringList files = QStringList(getValidFilePath() );

    monitor.setFiles(files);
    QStringList actualList= monitor.files();

    QCOMPARE(actualList.count(),1);
}

QString FileMonitorTest::getValidFilePath()
{
    QString file(QLatin1String("/tmp/"));
    file.append(getRandomString(8));
    return file;
}

QString FileMonitorTest::getRandomString(int length) const
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   // assuming you want random strings of 12 characters

   QString randomString;
   for(int i=0; i<length; ++i)
   {
       int index = qrand() % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}


QTEST_MAIN(FileMonitorTest)
