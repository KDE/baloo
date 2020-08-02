/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_PROPERTYDATA_H
#define BALOO_PROPERTYDATA_H

#include <KFileMetaData/Properties>
#include <QJsonObject>

namespace Baloo
{

const QJsonObject propertyMapToJson(const KFileMetaData::PropertyMap& properties);
const KFileMetaData::PropertyMap jsonToPropertyMap(const QJsonObject& properties);

} // namespace Baloo

#endif // BALOO_PROPERTYDATA_H
