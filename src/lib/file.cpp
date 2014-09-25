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
    d->rating = 0;
}

File::File(const File& f)
    : d(new FilePrivate(*f.d))
{
}

File::File(const QString& url)
    : d(new FilePrivate)
{
    d->url = url;
    d->rating = 0;
}

File::~File()
{
    delete d;
}

const File& File::operator=(const File& f)
{
    delete d;
    d = new FilePrivate(*f.d);
    return *this;
}

File File::fromId(const QByteArray& id)
{
    File file;
    file.setId(id);
    return file;
}

QByteArray File::id() const
{
    return d->id;
}

void File::setId(const QByteArray& id)
{
    d->id = id;
}

QString File::url() const
{
    return d->url;
}

void File::setUrl(const QString& url)
{
    d->url = url;
}

KFileMetaData::PropertyMap File::properties() const
{
    return d->propertyMap;
}

QVariant File::property(KFileMetaData::Property::Property property) const
{
    return d->propertyMap.value(property);
}
