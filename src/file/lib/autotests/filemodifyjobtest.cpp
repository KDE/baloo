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

#include "filemodifyjobtest.h"
#include "filemodifyjob.h"
#include "xattrdetector.h"
#include "../baloo_xattr_p.h"
#include "../db.h"
#include "filemapping.h"
#include "file.h"

#include <QDebug>
#include <QFileInfo>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include <xapian.h>

using namespace Baloo;

void FileModifyJobTest::testSingleFile()
{
    QTemporaryFile tmpFile(QLatin1String(BUILDDIR "testSingleFile.XXXXXX"));
    tmpFile.open();

    QTextStream stream(&tmpFile);
    stream << "ABC";

    const QString fileUrl = tmpFile.fileName();

    int fileSize = QFileInfo(fileUrl).size();

    File file(fileUrl);
    file.setRating(5);
    file.setUserComment(QLatin1String("User Comment"));

    FileModifyJob* job = new FileModifyJob(file);
    QVERIFY(job->exec());

    int fileSizeNew = QFileInfo(fileUrl).size();
    QCOMPARE(fileSize, fileSizeNew);

    QString value;

    XattrDetector detector;
    if (detector.isSupported(tmpFile.fileName())) {
        int len = baloo_getxattr(fileUrl, QLatin1String("user.baloo.rating"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);

        len = baloo_getxattr(fileUrl, QLatin1String("user.xdg.tags"), &value);
        QCOMPARE(len, -1);

        len = baloo_getxattr(fileUrl, QLatin1String("user.xdg.comment"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value, QLatin1String("User Comment"));
    }
    else {
        qWarning() << "Xattr not supported on this filesystem";
    }
}

void FileModifyJobTest::testMultiFileRating()
{
    QTemporaryFile tmpFile1(QLatin1String(BUILDDIR "testMultiFileRating.XXXXXX"));
    tmpFile1.open();

    QTemporaryFile tmpFile2(QLatin1String(BUILDDIR "testMultiFileRating.XXXXXX"));
    tmpFile2.open();

    const QString fileUrl1 = tmpFile1.fileName();
    const QString fileUrl2 = tmpFile2.fileName();

    QStringList files;
    files << fileUrl1 << fileUrl2;

    FileModifyJob* job = FileModifyJob::modifyRating(files, 5);
    QVERIFY(job->exec());

    QString value;

    XattrDetector detector;
    if (detector.isSupported(tmpFile1.fileName())) {
        int len = baloo_getxattr(fileUrl1, QLatin1String("user.baloo.rating"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);

        len = baloo_getxattr(fileUrl2, QLatin1String("user.baloo.rating"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);
    }
    else {
        qWarning() << "Xattr not supported on this filesystem";
    }
}

void FileModifyJobTest::testFolder()
{
    QTemporaryDir dir;
    const QString url = dir.path();

    File file(url);
    file.setRating(5);

    FileModifyJob* job = new FileModifyJob(file);
    QVERIFY(job->exec());

    QString value;

    XattrDetector detector;
    if (detector.isSupported(dir.path())) {
        int len = baloo_getxattr(url, QLatin1String("user.baloo.rating"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);
    }
    else {
        qWarning() << "Xattr not supported on this filesystem";
    }
}


QTEST_MAIN(FileModifyJobTest)
