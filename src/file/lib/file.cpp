/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "file.h"
#include "file_p.h"

using namespace Baloo;

File::File()
    : d(new FilePrivate)
{
}

File::File(const QString& url)
    : d(new FilePrivate)
{
    d->url = url;
}

File::~File()
{
    delete d;
}

File File::fromId(const Item::Id& id)
{
    File file;
    file.setId(id);
    return file;
}

QString File::url() const
{
    return d->url;
}

QVariantMap File::properties() const
{
    return d->variantMap;
}

QVariant File::property(const QString& key) const
{
    return d->variantMap.value(key);
}
