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

#include "filemonitor.h"

#include <QTest>
#include <QSignalSpy>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Baloo;

class FileMonitorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void init();
    void cleanup();
    void testAddFileShouldReturnOneFileIfOneFileAdded();
    void testAddFileShouldReturnTwoFilesIfTwoFilesAdded();
    void testAddFileShouldRemoveTailingSlash();
    void testAddFileShouldNotAddNotLocalUrl();
    void testAddFileShouldAddLocalUrl();
    void testClearIfClearAfterOneFileAddedFilesShouldReturn0Items();
    void testSetFilesIfSetFilesWithOneElementFilesShouldReturn1Item();

private:
    QString getRandomValidFilePath();
    QString getRandomValidWebUrl();
    QString getRandomString(int length) const;
    FileMonitor* m_sut;

};

void FileMonitorTest::init()
{
    m_sut = new FileMonitor();
}

void FileMonitorTest::cleanup()
{
    delete m_sut;
}

void FileMonitorTest::test()
{
    QString file = getRandomValidFilePath();
    m_sut->addFile(file);

    QSignalSpy spy(m_sut, SIGNAL(fileMetaDataChanged(QString)));

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/files"),
                           QStringLiteral("org.kde"),
                           QStringLiteral("changed"));

    QList<QString> list;
    list << file;

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(list);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    QEventLoop loop;
    connect(m_sut, &FileMonitor::fileMetaDataChanged, &loop, &QEventLoop::quit);
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
    QString filePath = getRandomValidFilePath();
    m_sut->addFile(filePath);

    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 1);
    QCOMPARE(actualList.first(), filePath);
}

void FileMonitorTest::testAddFileShouldReturnTwoFilesIfTwoFilesAdded()
{
    QString filePath1 = getRandomValidFilePath();
    QString filePath2 = getRandomValidFilePath();
    m_sut->addFile(filePath1);
    m_sut->addFile(filePath2);

    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 2);
    QVERIFY(actualList.contains(filePath1));
    QVERIFY(actualList.contains(filePath2));
}

void FileMonitorTest::testAddFileShouldRemoveTailingSlash()
{
    QString expectedFilePath = getRandomValidFilePath();
    QString filePath(expectedFilePath);
    filePath.append("/");

    m_sut->addFile(filePath);

    QStringList actualList = m_sut->files();
    QCOMPARE(actualList.first(), expectedFilePath);
}

void FileMonitorTest::testAddFileShouldNotAddNotLocalUrl()
{
    QUrl fileUrl(getRandomValidWebUrl());

    m_sut->addFile(fileUrl);
    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 0);
}

void FileMonitorTest::testAddFileShouldAddLocalUrl()
{
    QUrl fileUrl(getRandomValidFilePath());

    m_sut->addFile(fileUrl);
    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 0);
}
void FileMonitorTest::testClearIfClearAfterOneFileAddedFilesShouldReturn0Items()
{
    QUrl fileUrl(getRandomValidFilePath());

    m_sut->addFile(fileUrl);

    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 0);
}
void FileMonitorTest::testSetFilesIfSetFilesWithOneElementFilesShouldReturn1Item()
{
    QStringList files = QStringList(getRandomValidFilePath());

    m_sut->setFiles(files);
    QStringList actualList = m_sut->files();

    QCOMPARE(actualList.count(), 1);
}

QString FileMonitorTest::getRandomValidWebUrl()
{
    QString file = "http://" + getRandomString(4) + ".com/" + getRandomString(4);
    return file;
}

QString FileMonitorTest::getRandomValidFilePath()
{
    QString file(QStringLiteral("/tmp/"));
    file.append(getRandomString(8));
    return file;
}

QString FileMonitorTest::getRandomString(int length) const
{
    const QString possibleCharacters(QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
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


QTEST_GUILESS_MAIN(FileMonitorTest)

#include "filemonitortest.moc"
