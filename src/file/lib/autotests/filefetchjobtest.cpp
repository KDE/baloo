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
#include "xattrdetector.h"
#include "../baloo_xattr_p.h"
#include "../db.h"
#include "filemapping.h"
#include "file.h"

#include <QSqlQuery>
#include <QDebug>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include <QJsonDocument>
#include <QJsonObject>

#include <xapian.h>

#include <KFileMetaData/Properties>

using namespace Baloo;

void FileFetchJobTest::init()
{
    // TODO: Remove the old xapian DB?
    // Create the Xapian DB
    const std::string xapianPath = fileIndexDbPath();
    Xapian::WritableDatabase db(xapianPath, Xapian::DB_CREATE_OR_OPEN);

    // Clear the sqlite db
    QSqlDatabase sqlDb = fileMappingDb();
    sqlDb.exec(QLatin1String("delete from files"));
}

void FileFetchJobTest::testXapianData()
{
    using namespace KFileMetaData;
    PropertyMap map;
    map.insert(Property::Album, QLatin1String("value1"));
    map.insert(Property::Artist, QLatin1String("value2"));

    QJsonObject jo = QJsonObject::fromVariantMap(toVariantMap(map));
    QJsonDocument jdoc;
    jdoc.setObject(jo);

    QByteArray json = jdoc.toJson();
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

    QString fileName = tempFile.fileName();
    /*
    XattrDetector detector;
    if (!detector.isSupported(fileName)) {
        qWarning() << "Xattr not supported on this filesystem";
        return;
    }*/

    FileMapping fileMap(tempFile.fileName());
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.create(sqlDb));

    QString rat = QString::number(7);
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.baloo.rating"), rat) != -1);

    QStringList tags;
    tags << QLatin1String("TagA") << QLatin1String("TagB");

    QString tagStr = tags.join(QLatin1String(","));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.tags"), tagStr) != -1);

    const QString userComment(QLatin1String("UserComment"));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), userComment) != -1);

    FileFetchJob* job = new FileFetchJob(tempFile.fileName());
    job->exec();
    File file = job->file();

    QCOMPARE(file.rating(), 7);
    QCOMPARE(file.tags(), tags);
    QCOMPARE(file.userComment(), userComment);
}

void FileFetchJobTest::testFolder()
{
    // We use the same prefix as the tmpfile
    QTemporaryDir tmpDir;
    QString fileName = tmpDir.path();

    XattrDetector detector;
    if (!detector.isSupported(fileName)) {
        qWarning() << "Xattr not supported on this filesystem";
        return;
    }

    FileMapping fileMap(fileName);
    QSqlDatabase sqlDb = fileMappingDb();
    QVERIFY(fileMap.create(sqlDb));

    QString rat = QString::number(7);
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.baloo.rating"), rat) != -1);

    QStringList tags;
    tags << QLatin1String("TagA") << QLatin1String("TagB");

    QString tagStr = tags.join(QLatin1String(","));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.tags"), tagStr) != -1);

    const QString userComment(QLatin1String("UserComment"));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), userComment) != -1);

    FileFetchJob* job = new FileFetchJob(fileName);
    job->exec();
    File file = job->file();

    QCOMPARE(file.rating(), 7);
    QCOMPARE(file.tags(), tags);
    QCOMPARE(file.userComment(), userComment);
}

QTEST_MAIN(FileFetchJobTest)
