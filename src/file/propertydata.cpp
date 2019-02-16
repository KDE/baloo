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
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

namespace Baloo
{

const QJsonObject propertyMapToJson(const KFileMetaData::PropertyMap& properties)
{
    auto it = properties.cbegin();
    QJsonObject jsonDict;

    while (it != properties.cend()) {

        auto property = it.key();
        QString keyString = QString::number(static_cast<int>(property));

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

const KFileMetaData::PropertyMap jsonToPropertyMap(const QJsonObject& properties)
{
    KFileMetaData::PropertyMap propertyMap;

    auto it = properties.begin();
    while (it != properties.end()) {
        int propNum = it.key().toInt();
        auto prop = static_cast<KFileMetaData::Property::Property>(propNum);

        if (it.value().isArray()) {
            const auto values = it.value().toArray();
            for (const auto val : values) {
                propertyMap.insertMulti(prop, val.toVariant());
            }

        } else if (it.value().isDouble()) {
            propertyMap.insertMulti(prop, it.value().toDouble());
        } else {
            propertyMap.insertMulti(prop, it.value().toString());
        }
        ++it;
    }

    return propertyMap;
}

} // namespace Baloo
