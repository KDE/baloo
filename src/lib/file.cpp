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
#include "global.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDebug>

using namespace Baloo;

class File::Private {
public:
    QString url;
    KFileMetaData::PropertyMap propertyMap;
};

File::File()
    : d(new Private)
{
}

File::File(const File& f)
    : d(new Private(*f.d))
{
}

File::File(const QString& url)
    : d(new Private)
{
    d->url = QFileInfo(url).canonicalFilePath();
}

File::~File()
{
    delete d;
}

const File& File::operator=(const File& f)
{
    delete d;
    d = new Private(*f.d);
    return *this;
}

QString File::path() const
{
    return d->url;
}

KFileMetaData::PropertyMap File::properties() const
{
    return d->propertyMap;
}

QVariant File::property(KFileMetaData::Property::Property property) const
{
    return d->propertyMap.value(property);
}

bool File::load(const QString& url)
{
    d->url = QFileInfo(url).canonicalFilePath();
    return load();
}

bool File::load()
{
    const QString& url = d->url;
    if (url.isEmpty() || !QFile::exists(url)) {
        return false;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadOnlyDatabase)) {
        return false;
    }

    quint64 id = filePathToId(QFile::encodeName(d->url));
    if (!id) {
        return false;
    }

    QByteArray arr;
    {
        Transaction tr(db, Transaction::ReadOnly);
        arr = tr.documentData(id);
    }
    if (arr.isEmpty()) {
        return false;
    }

    const QJsonDocument jdoc = QJsonDocument::fromJson(arr);
    const QVariantMap varMap = jdoc.object().toVariantMap();
    d->propertyMap = KFileMetaData::toPropertyMap(varMap);

    return true;
}
