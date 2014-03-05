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

#include "filefetchjobtest.h"
#include "filefetchjob.h"
#include "../db.h"
#include "filemapping.h"
#include "file.h"

#include "qtest_kde.h"
#include <QSqlQuery>
#include <KTempDir>
#include <KDebug>

#include <qjson/serializer.h>
#include <xapian.h>
#include <attr/xattr.h>

#include <kfilemetadata/properties.h>

using namespace Baloo;

void FileFetchJobTest::init()
{
    // TODO: Remove the old xapian DB?
    // Create the Xapian DB
    const std::string xapianPath = fileIndexDbPath();
    Xapian::WritableDatabase db(xapianPath, Xapian::DB_CREATE_OR_OPEN);

    // Clear the sqlite db
    QSqlDatabase sqlDb = fileMappingDb();
    sqlDb.exec("delete from files");
}

void FileFetchJobTest::testXapianData()
{
    QJson::Serializer serializer;

    using namespace KFileMetaData;
    PropertyMap map;
    map.insert(Property::Album, "value1");
    map.insert(Property::Artist, "value2");

    QByteArray json = serializer.serialize(toVariantMap(map));
    QVERIFY(!json.isEmpty());

    Xapian::Document doc;
    doc.set_data(json.constData());

    QTemporaryFile tempFile;
    tempFile.open();

    FileMapping fileMap(tempFile.fileName());
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.create(sqlDb));

    {
        const std::string xapianPath = fileIndexDbPath();
        Xapian::WritableDatabase db(xapianPath, Xapian::DB_CREATE_OR_OPEN);
        db.replace_document(fileMap.id(), doc);
        db.commit();
    }

    FileFetchJob* job = new FileFetchJob(tempFile.fileName());
    job->exec();
    File file = job->file();

    QCOMPARE(file.properties(), map);
    QCOMPARE(file.rating(), 0);
    QVERIFY(file.tags().isEmpty());
    QVERIFY(file.userComment().isEmpty());
}

void FileFetchJobTest::testExtendedAttributes()
{
    QTemporaryFile tempFile(QLatin1String(BUILDDIR "testExtendedAttributes.XXXXXX"));
    tempFile.open();
    QByteArray fileName = QFile::encodeName(tempFile.fileName());

    FileMapping fileMap(tempFile.fileName());
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.create(sqlDb));

    QByteArray rat = QString::number(7).toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.baloo.rating", rat.constData(), rat.size(), 0) != -1);

    QStringList tags;
    tags << "TagA" << "TagB";

    QByteArray tagStr = tags.join(",").toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.xdg.tags", tagStr.constData(), tagStr.size(), 0) != -1);

    const QString userComment("UserComment");
    QByteArray com = userComment.toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.xdg.comment", com.constData(), com.size(), 0) != -1);

    FileFetchJob* job = new FileFetchJob(tempFile.fileName());
    job->exec();
    File file = job->file();

    QCOMPARE(file.rating(), 7);
    QCOMPARE(file.tags(), tags);
    QCOMPARE(file.userComment(), userComment);
}

void FileFetchJobTest::testFolder()
{
    QTemporaryFile f(QLatin1String(BUILDDIR "testFolder.XXXXXX"));
    f.open();

    // We use the same prefix as the tmpfile
    KTempDir tmpDir(f.fileName().mid(0, f.fileName().lastIndexOf('/') + 1));
    QByteArray fileName = QFile::encodeName(tmpDir.name());

    FileMapping fileMap(tmpDir.name());
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.create(sqlDb));

    QByteArray rat = QString::number(7).toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.baloo.rating", rat.constData(), rat.size(), 0) != -1);

    QStringList tags;
    tags << "TagA" << "TagB";

    QByteArray tagStr = tags.join(",").toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.xdg.tags", tagStr.constData(), tagStr.size(), 0) != -1);

    const QString userComment("UserComment");
    QByteArray com = userComment.toUtf8();
    QVERIFY(setxattr(fileName.constData(), "user.xdg.comment", com.constData(), com.size(), 0) != -1);

    FileFetchJob* job = new FileFetchJob(tmpDir.name());
    job->exec();
    File file = job->file();

    QCOMPARE(file.rating(), 7);
    QCOMPARE(file.tags(), tags);
    QCOMPARE(file.userComment(), userComment);
}

QTEST_KDEMAIN_CORE(FileFetchJobTest)
