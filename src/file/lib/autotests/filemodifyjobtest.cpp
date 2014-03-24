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
#include "../baloo_xattr_p.h"
#include "../xattrdetector.h"
#include "../db.h"
#include "filemapping.h"
#include "file.h"

#include "qtest_kde.h"
#include <KDebug>
#include <KTempDir>

#include <qjson/serializer.h>
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
    file.setUserComment("User Comment");

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

        len = baloo_getxattr(fileUrl, "user.xdg.tags", &value);
        QCOMPARE(len, 0);

        len = baloo_getxattr(fileUrl, "user.xdg.comment", &value);
        QVERIFY(len > 0);
        QCOMPARE(value, QString("User Comment"));
    }
    else {
        kWarning() << "Xattr not supported on this filesystem";
    }

    //
    // Check in Xapian
    //
    FileMapping fileMap(fileUrl);
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.fetch(sqlDb));

    const std::string xapianPath = fileIndexDbPath();
    Xapian::Database db(xapianPath);

    Xapian::Document doc = db.get_document(fileMap.id());

    Xapian::TermIterator iter = doc.termlist_begin();
    QCOMPARE(*iter, std::string("Ccomment"));

    iter++;
    QCOMPARE(*iter, std::string("Cuser"));

    iter++;
    QCOMPARE(*iter, std::string("R5"));
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
        int len = baloo_getxattr(fileUrl1, "user.baloo.rating", &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);

        len = baloo_getxattr(fileUrl2, "user.baloo.rating", &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);
    }
    else {
        kWarning() << "Xattr not supported on this filesystem";
    }
}

void FileModifyJobTest::testXapianUpdate()
{
    QTemporaryFile tmpFile;
    tmpFile.open();
    const QString fileUrl = tmpFile.fileName();

    File file(fileUrl);
    file.setRating(4);
    file.addTag("Round-Tag");

    FileModifyJob* job = new FileModifyJob(file);
    QVERIFY(job->exec());

    FileMapping fileMap(fileUrl);
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.fetch(sqlDb));

    const std::string xapianPath = fileIndexDbPath();
    Xapian::Database db(xapianPath);

    Xapian::Document doc = db.get_document(fileMap.id());

    Xapian::TermIterator iter = doc.termlist_begin();
    QCOMPARE(*iter, std::string("R4"));
    iter++;
    QCOMPARE(*iter, std::string("TAG-Round-Tag"));
    iter++;
    QCOMPARE(*iter, std::string("TAround"));
    iter++;
    QCOMPARE(*iter, std::string("TAtag"));
    iter++;
    QCOMPARE(iter, doc.termlist_end());

    // Add another term, and make sure it is not removed
    doc.add_term("RATING");
    {
        const std::string xapianPath = fileIndexDbPath();
        Xapian::WritableDatabase db(xapianPath, Xapian::DB_CREATE_OR_OPEN);
        db.replace_document(fileMap.id(), doc);
        db.commit();
    }

    file.setRating(5);
    file.setTags(QStringList() << "Square-Tag");
    job = new FileModifyJob(file);
    QVERIFY(job->exec());

    db.reopen();
    doc = db.get_document(fileMap.id());

    iter = doc.termlist_begin();
    QCOMPARE(*iter, std::string("R5"));
    iter++;
    QCOMPARE(*iter, std::string("RATING"));
    iter++;
    QCOMPARE(*iter, std::string("TAG-Square-Tag"));
    iter++;
    QCOMPARE(*iter, std::string("TAsquare"));
    iter++;
    QCOMPARE(*iter, std::string("TAtag"));
    iter++;
    QCOMPARE(iter, doc.termlist_end());
}

void FileModifyJobTest::testFolder()
{
    QTemporaryFile f(QLatin1String(BUILDDIR "testFolder.XXXXXX"));
    f.open();

    // We use the same prefix as the tmpfile
    KTempDir dir(f.fileName().mid(0, f.fileName().lastIndexOf('/') + 1));
    const QString url = dir.name();

    File file(url);
    file.setRating(5);

    FileModifyJob* job = new FileModifyJob(file);
    QVERIFY(job->exec());

    QString value;

    XattrDetector detector;
    if (detector.isSupported(f.fileName())) {
        int len = baloo_getxattr(url, QLatin1String("user.baloo.rating"), &value);
        QVERIFY(len > 0);
        QCOMPARE(value.toInt(), 5);
    }
    else {
        kWarning() << "Xattr not supported on this filesystem";
    }
}


QTEST_KDEMAIN_CORE(FileModifyJobTest)
