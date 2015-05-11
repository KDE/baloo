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

#include "fileindexingjobtest.h"
#include "fileindexingjob.h"

#include <QSignalSpy>
#include <QTest>

using namespace Baloo;

void FileIndexingJobTest::init()
{
    // Set the correct path
    QByteArray path = qgetenv("PATH");
#ifndef Q_OS_WIN
    path = QByteArray(BALOO_TEMP_PATH) + ":" + path;
#else
    path = QByteArray(BALOO_TEMP_PATH) + ";" + path;
#endif

    qputenv("PATH", path);
    qunsetenv("BALOO_EXTRACTOR_FAIL_FILE");
}

void FileIndexingJobTest::testFileFail()
{
    QVector<quint64> files;
    for (int i = 0; i < 40; ++i) {
        files << i;
    }

    qputenv("BALOO_EXTRACTOR_FAIL_FILE", QByteArray("5"));
    FileIndexingJob* job = new FileIndexingJob(files);

    QSignalSpy spy(job, SIGNAL(indexingFailed(quint64)));
    QVERIFY(job->exec());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().toULongLong(), (quint64)5);
}

void FileIndexingJobTest::testMultiFileFail()
{
    QVector<quint64> files;
    for (int i = 0; i < 40; ++i) {
        files << i;
    }

    qputenv("BALOO_EXTRACTOR_FAIL_FILE", QByteArray("5,18,19"));
    FileIndexingJob* job = new FileIndexingJob(files);

    QSignalSpy spy(job, SIGNAL(indexingFailed(quint64)));
    QVERIFY(job->exec());

    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().toULongLong(), (quint64)5);
    QCOMPARE(spy.at(1).size(), 1);
    QCOMPARE(spy.at(1).first().toULongLong(), (quint64)18);
    QCOMPARE(spy.at(2).size(), 1);
    QCOMPARE(spy.at(2).first().toULongLong(), (quint64)19);
}

void FileIndexingJobTest::testNormalExecution()
{
    QVector<quint64> files;
    files << 1 << 2 << 3 << 4 << 5 << 6;

    FileIndexingJob* job = new FileIndexingJob(files);

    QSignalSpy spy1(job, SIGNAL(indexingFailed(quint64)));
    QSignalSpy spy2(job, SIGNAL(finished(KJob*)));
    QVERIFY(job->exec());

    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 1);
}

void FileIndexingJobTest::testTimeout()
{
    QVector<quint64> files;
    for (int i = 0; i<40; ++i) {
        files << i;
    }

    qputenv("BALOO_EXTRACTOR_TIMEOUT_FILE", QByteArray("5"));
    FileIndexingJob* job = new FileIndexingJob(files);
    job->setTimeoutInterval(100);

    QSignalSpy spy(job, SIGNAL(indexingFailed(quint64)));
    QVERIFY(job->exec());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).first().toULongLong(), (quint64)5);
}


QTEST_MAIN(FileIndexingJobTest);
