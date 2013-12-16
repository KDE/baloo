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

#include "filemapping.h"

#include <QSqlQuery>
#include <QVariant>

using namespace Baloo;

class FileMapping::Private {
public:
    Private() : id(0) {}

    QString url;
    uint id;
};

FileMapping::FileMapping()
    : d(new Private)
{
}

FileMapping::FileMapping(const QString& url)
    : d(new Private)
{
    d->url = url;
}

FileMapping::FileMapping(uint id)
    : d(new Private)
{
    d->id = id;
}

uint FileMapping::id() const
{
    return d->id;
}

QString FileMapping::url() const
{
    return d->url;
}

void FileMapping::setId(uint id)
{
    d->id = id;
}

void FileMapping::setUrl(const QString& url)
{
    d->url = url;
}

bool FileMapping::fetched()
{
    if (d->id == 0 || d->url.isEmpty())
        return false;

    return true;
}

bool FileMapping::fetch(QSqlDatabase db)
{
    if (fetched())
        return true;

    if (d->id == 0 && d->url.isEmpty())
        return false;

    if (d->url.isEmpty()) {
        QSqlQuery query(db);
        query.setForwardOnly(true);
        query.prepare(QLatin1String("select url from files where id = ?"));
        query.addBindValue(d->id);
        query.exec();

        if (!query.next()) {
            return false;
        }

        d->url = query.value(0).toString();
    }
    else {
        QSqlQuery query(db);
        query.setForwardOnly(true);
        query.prepare(QLatin1String("select id from files where url = ?"));
        query.addBindValue(d->url);
        query.exec();

        if (!query.next()) {
            return false;
        }

        d->id = query.value(0).toUInt();
    }

    return true;
}

bool FileMapping::create(QSqlDatabase db)
{
    if (d->id)
        return false;

    if (d->url.isEmpty())
        return false;

    QSqlQuery query(db);
    query.prepare(QLatin1String("insert into files (url) VALUES (?)"));
    query.addBindValue(d->url);

    if (!query.exec()) {
        return false;
    }

    d->id = query.lastInsertId().toUInt();
    return true;
}

void FileMapping::clear()
{
    d->id = 0;
    d->url.clear();
}

bool FileMapping::empty() const
{
    return d->url.isEmpty() && d->id == 0;
}

bool FileMapping::operator==(const FileMapping& rhs) const
{
    if (rhs.empty() && empty())
        return true;

    if (!rhs.url().isEmpty() && !url().isEmpty())
        return rhs.url() == url();

    if (rhs.id() && id())
        return rhs.id() == id();

    return false;
}

