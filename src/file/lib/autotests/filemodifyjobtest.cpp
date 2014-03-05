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
#include "../db.h"
#include "filemapping.h"
#include "file.h"

#include "qtest_kde.h"
#include <KDebug>
#include <KTempDir>

#include <qjson/serializer.h>
#include <xapian.h>
#include <attr/xattr.h>

using namespace Baloo;

void FileModifyJobTest::testSingleFile()
{
    QTemporaryFile tmpFile;
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

    char buffer[1000];
    const QByteArray arr = QFile::encodeName(fileUrl);

    int len = getxattr(arr.constData(), "user.baloo.rating", &buffer, 1000);
    QVERIFY(len > 0);

    int r = QString::fromUtf8(buffer, len).toInt();
    QCOMPARE(r, 5);

    len = getxattr(arr.constData(), "user.xdg.tags", &buffer, 1000);
    QCOMPARE(len, -1);

    len = getxattr(arr.constData(), "user.xdg.comment", &buffer, 1000);
    QVERIFY(len > 0);
    QString comment = QString::fromUtf8(buffer, len);
    QCOMPARE(comment, QString("User Comment"));

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
    QTemporaryFile tmpFile1;
    tmpFile1.open();

    QTemporaryFile tmpFile2;
    tmpFile2.open();

    const QString fileUrl1 = tmpFile1.fileName();
    const QString fileUrl2 = tmpFile2.fileName();

    QStringList files;
    files << fileUrl1 << fileUrl2;

    FileModifyJob* job = FileModifyJob::modifyRating(files, 5);
    QVERIFY(job->exec());

    char buffer[1000];

    const QByteArray arr1 = QFile::encodeName(fileUrl1);
    int len = getxattr(arr1.constData(), "user.baloo.rating", &buffer, 1000);
    QVERIFY(len > 0);

    int r = QString::fromUtf8(buffer, len).toInt();
    QCOMPARE(r, 5);

    const QByteArray arr2 = QFile::encodeName(fileUrl2);
    len = getxattr(arr2.constData(), "user.baloo.rating", &buffer, 1000);
    QVERIFY(len > 0);

    r = QString::fromUtf8(buffer, len).toInt();
    QCOMPARE(r, 5);
}

void FileModifyJobTest::testXapianUpdate()
{
    QTemporaryFile tmpFile;
    tmpFile.open();
    const QString fileUrl = tmpFile.fileName();

    File file(fileUrl);
    file.setRating(4);

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
    job = new FileModifyJob(file);
    QVERIFY(job->exec());

    db.reopen();
    doc = db.get_document(fileMap.id());

    iter = doc.termlist_begin();
    QCOMPARE(*iter, std::string("R5"));
    iter++;
    QCOMPARE(*iter, std::string("RATING"));
    iter++;
    QCOMPARE(iter, doc.termlist_end());
}

void FileModifyJobTest::testFolder()
{
    QTemporaryFile f;
    f.open();

    // We use the same prefix as the tmpfile
    KTempDir dir(f.fileName().mid(0, f.fileName().lastIndexOf('/') + 1));
    const QString url = dir.name();

    File file(url);
    file.setRating(5);

    FileModifyJob* job = new FileModifyJob(file);
    QVERIFY(job->exec());

    char buffer[1000];

    const QByteArray arr1 = QFile::encodeName(url);
    int len = getxattr(arr1.constData(), "user.baloo.rating", &buffer, 1000);
    QVERIFY(len > 0);

    int r = QString::fromUtf8(buffer, len).toInt();
    QCOMPARE(r, 5);
}


QTEST_KDEMAIN_CORE(FileModifyJobTest)
