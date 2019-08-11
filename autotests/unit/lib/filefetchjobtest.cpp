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

#include "file.h"
#include "document.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"
#include "global.h"

#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include <QJsonDocument>
#include <QJsonObject>


using namespace Baloo;

class FileFetchJobTest : public QObject
{
    Q_OBJECT

    QTemporaryDir dir;

private Q_SLOTS:
    void test();
};

void FileFetchJobTest::test()
{
    using namespace KFileMetaData;

    setenv("BALOO_DB_PATH", dir.path().toStdString().c_str(), 1);

    PropertyMap map;
    map.insert(Property::Album, QLatin1String("value1"));
    map.insert(Property::Artist, QLatin1String("value2"));

    QJsonObject jo = QJsonObject::fromVariantMap(toVariantMap(map));
    QJsonDocument jdoc;
    jdoc.setObject(jo);

    QByteArray json = jdoc.toJson();
    QVERIFY(!json.isEmpty());

    QTemporaryFile tempFile;
    tempFile.open();

    Document doc;
    doc.setData(json);
    doc.setUrl(tempFile.fileName().toUtf8());
    doc.setId(filePathToId(doc.url()));
    doc.addTerm("testterm");
    doc.addFileNameTerm("filename");
    doc.setMTime(1);
    doc.setCTime(1);

    {
        Database db(fileIndexDbPath());
        db.open(Database::CreateDatabase);

        Transaction tr(db, Transaction::ReadWrite);
        tr.addDocument(doc);
        tr.commit();
    }

    File file(tempFile.fileName());
    QVERIFY(file.load());
    QCOMPARE(file.properties(), map);
}

QTEST_MAIN(FileFetchJobTest)

#include "filefetchjobtest.moc"
