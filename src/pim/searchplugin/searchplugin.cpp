/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "searchplugin.h"

#include <query.h>

#include <KDebug>
#include <QtPlugin>

QSet<qint64> SearchPlugin::search(const QString& queryString)
{
    QSet<qint64> resultSet;

    Baloo::Query query = Baloo::Query::fromJSON(queryString.toLatin1());
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QByteArray id = iter.id();
        const int fid = Baloo::deserialize("akonadi", id);
        resultSet << fid;
    }
    return resultSet;
}

Q_EXPORT_PLUGIN2(akonadi_baloo_searchplugin, SearchPlugin)
