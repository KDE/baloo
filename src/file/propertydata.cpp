/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "propertydata.h"
#include <QJsonArray>
#include <QJsonValue>

namespace Baloo
{

const QJsonObject propertyMapToJson(const KFileMetaData::PropertyMultiMap& properties)
{
    auto it = properties.cbegin();
    QJsonObject jsonDict;

    while (it != properties.cend()) {
        auto property = it.key();
        QString keyString = QString::number(static_cast<int>(property), 10);

        auto rangeEnd = properties.upperBound(property);

        QJsonValue value;
        // In case a key has multiple values, convert to QJsonArray
        if (std::distance(it, rangeEnd) > 1) {
            QJsonArray values;

            // Last inserted is first element, for backwards compatible
            // ordering prepend earlier elements
            while (it != rangeEnd) {
                values.insert(0, QJsonValue::fromVariant(it.value()));
                ++it;
            };

            value = values;
        } else {
            auto type = it.value().userType();
            if ((type == QMetaType::QVariantList) || (type == QMetaType::QStringList)) {
                // if it is a QList<T>, recurse
                auto list = it.value().toList();
                QJsonArray values;
                while (!list.isEmpty()) {
                    values.push_back(QJsonValue::fromVariant(list.takeLast()));
                }
                value = values;
            } else {
                value = QJsonValue::fromVariant(it.value());
            }
        }

        jsonDict.insert(keyString, value);

        // pivot to next key
        it = rangeEnd;
    }

    return jsonDict;
}

const KFileMetaData::PropertyMultiMap jsonToPropertyMap(const QJsonObject& properties)
{
    KFileMetaData::PropertyMultiMap propertyMap;

    for (auto [key, value] : properties.asKeyValueRange()) {
        bool isInt = true;
        int propNum = key.toString().toInt(&isInt, 10);
        if (!isInt) {
            continue;
        }

        auto prop = static_cast<KFileMetaData::Property::Property>(propNum);

        if (value.isArray()) {
            const auto values = value.toArray();
            for (const auto val : values) {
                propertyMap.insert(prop, val.toVariant());
            }

        } else if (value.isDouble()) {
            propertyMap.insert(prop, value.toDouble());
        } else {
            propertyMap.insert(prop, value.toString());
        }
    }

    return propertyMap;
}

} // namespace Baloo
