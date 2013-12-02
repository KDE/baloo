/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "result.h"

using namespace Baloo;

class Result::Private {
public:
    QByteArray id;
    QString text;
    QString icon;
    QUrl url;
};

Result::Result()
    : d(new Private)
{
}

Result::Result(const Result& rhs)
    : d(new Private(*rhs.d))
{
}

Result::~Result()
{
    delete d;
}

void Result::setId(const QByteArray& id)
{
    d->id = id;
}

QByteArray Result::id() const
{
    return d->id;
}

void Result::setText(const QString& text)
{
    d->text = text;
}

QString Result::text() const
{
    return d->text;
}

QString Result::icon() const
{
    return d->icon;
}

void Result::setIcon(const QString& icon)
{
    d->icon = icon;
}

QUrl Result::url() const
{
    return d->url;
}

void Result::setUrl(const QUrl& url)
{
    d->url = url;
}

