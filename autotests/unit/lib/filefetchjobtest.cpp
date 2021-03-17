/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "document.h"
#include "file.h"
#include "global.h"
#include "idutils.h"
#include "transaction.h"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

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
