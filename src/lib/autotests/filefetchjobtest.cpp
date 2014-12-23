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
#include "../db.h"
#include "filemapping.h"
#include "file.h"

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

    doc.add_value(3, tempFile.fileName().toUtf8().constData());
    doc.add_boolean_term(("P-" + tempFile.fileName()).toUtf8().constData());

    {
        const std::string xapianPath = fileIndexDbPath();
        Xapian::WritableDatabase db(xapianPath, Xapian::DB_CREATE_OR_OPEN);
        db.add_document(doc);
        db.commit();
    }

    File file(tempFile.fileName());
    QVERIFY(file.load());

    QCOMPARE(file.properties(), map);
}


QTEST_MAIN(FileFetchJobTest)
