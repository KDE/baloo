/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    properties.insertMulti(KFMProp::Subject, QStringLiteral("subject"));

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
    properties.insertMulti(KFMProp::Subject, QStringLiteral("subject"));
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
    properties.insertMulti(KFMProp::Genre, QStringLiteral("genre1"));
    properties.insertMulti(KFMProp::Genre, QStringLiteral("genre2"));
    properties.insertMulti(KFMProp::Ensemble, QStringLiteral("ensemble1"));
    properties.insertMulti(KFMProp::Ensemble, QStringLiteral("ensemble2"));
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
    properties.insertMulti(KFMProp::Genre, QStringList({QStringLiteral("genre1"), QStringLiteral("genre2")}));
    properties.insertMulti(KFMProp::Ensemble, QVariantList({QStringLiteral("ensemble1"), QStringLiteral("ensemble2")}));
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
