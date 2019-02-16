/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2019  Stefan Brüns <stefan.bruens@rwth-aachen.de>
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

#include "propertydata.h"

#include <QTest>

// #include <QDebug>
// #include <QJsonDocument>

using namespace Baloo;

class PropertySerializationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEmpty();
    void testSingleProp();
    void testMultiProp();
    void testMultiValue();
    void testLists();
};

// Test empty property map
void PropertySerializationTest::testEmpty()
{
    KFileMetaData::PropertyMap properties;

    auto json = propertyMapToJson(properties);
    // QJsonDocument jdoc(json);
    // qDebug() << __func__ << jdoc.toJson(QJsonDocument::JsonFormat::Compact);

    auto res = jsonToPropertyMap(json);
    QCOMPARE(res, properties);
}

// Test serialization/deserialization a single property
void PropertySerializationTest::testSingleProp()
{
    namespace KFMProp = KFileMetaData::Property;

    KFileMetaData::PropertyMap properties;
    properties.insertMulti(KFMProp::Subject, "subject");

    auto json = propertyMapToJson(properties);
    // QJsonDocument jdoc(json);
    // qDebug() << __func__ << jdoc.toJson(QJsonDocument::JsonFormat::Compact);

    auto res = jsonToPropertyMap(json);
    QCOMPARE(res, properties);
}

// Test serialization/deserialization for multiple properties
void PropertySerializationTest::testMultiProp()
{
    namespace KFMProp = KFileMetaData::Property;

    KFileMetaData::PropertyMap properties;
    properties.insertMulti(KFMProp::Subject, "subject");
    properties.insertMulti(KFMProp::ReleaseYear, 2019);
    properties.insertMulti(KFMProp::PhotoExposureTime, 0.12345);

    auto json = propertyMapToJson(properties);
    // QJsonDocument jdoc(json);
    // qDebug() << __func__ << jdoc.toJson(QJsonDocument::JsonFormat::Compact);

    auto res = jsonToPropertyMap(json);

    for (auto prop : { KFMProp::Subject, KFMProp::ReleaseYear, KFMProp::PhotoExposureTime }) {
        QCOMPARE(res[prop], properties[prop]);
    }
    QCOMPARE(res, properties);
}

// Test serialization/deserialization for multiple value per property
void PropertySerializationTest::testMultiValue()
{
    namespace KFMProp = KFileMetaData::Property;

    KFileMetaData::PropertyMap properties;
    properties.insertMulti(KFMProp::Genre, "genre1");
    properties.insertMulti(KFMProp::Genre, "genre2");
    properties.insertMulti(KFMProp::Ensemble, "ensemble1");
    properties.insertMulti(KFMProp::Ensemble, "ensemble2");
    properties.insertMulti(KFMProp::Rating, 4);
    properties.insertMulti(KFMProp::Rating, 5);

    auto json = propertyMapToJson(properties);
    // QJsonDocument jdoc(json);
    // qDebug() << __func__ << jdoc.toJson(QJsonDocument::JsonFormat::Compact);

    auto res = jsonToPropertyMap(json);

    for (auto prop : { KFMProp::Genre, KFMProp::Ensemble, KFMProp::Rating}) {
        QCOMPARE(res[prop], properties[prop]);
        QCOMPARE(res.values(prop), properties.values(prop));
        // qDebug() << res[prop];
        // qDebug() << res.values(prop);
        // qDebug() << QVariant(res.values(prop)).toStringList().join(", ");
    }
    QCOMPARE(res, properties);
}

// Test serialization/deserialization of lists
void PropertySerializationTest::testLists()
{
    namespace KFMProp = KFileMetaData::Property;

    KFileMetaData::PropertyMap properties;
    properties.insertMulti(KFMProp::Genre, QStringList({"genre1", "genre2"}));
    properties.insertMulti(KFMProp::Ensemble, QVariantList({QVariant("ensemble1"), QVariant("ensemble2")}));
    properties.insertMulti(KFMProp::Rating, QVariantList({QVariant(10), QVariant(7.7)}));

    auto json = propertyMapToJson(properties);
    // QJsonDocument jdoc(json);
    // qDebug() << __func__ << jdoc.toJson(QJsonDocument::JsonFormat::Compact);

    auto res = jsonToPropertyMap(json);

    for (auto prop : { KFMProp::Genre, KFMProp::Ensemble, KFMProp::Rating}) {
        // qDebug() << res.values(prop);
        // qDebug() << QVariant(res.values(prop)).toStringList().join(", ");
        // qDebug() << properties[prop].toStringList().join(", ");
        QCOMPARE(QVariant(res.values(prop)).toStringList(), properties[prop].toStringList());
    }
}


QTEST_MAIN(PropertySerializationTest)

#include "propertyserializationtest.moc"
