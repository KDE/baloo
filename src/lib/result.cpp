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

#include "result.h"

using namespace Baloo;

class Result::Private {
public:
    uint id;
    QString filePath;
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

void Result::setId(const uint& id)
{
    d->id = id;
}

uint Result::id() const
{
    return d->id;
}

QString Result::filePath() const
{
    return d->filePath;
}

void Result::setFilePath(const QString& path)
{
    d->filePath = path;
}

Result& Result::operator=(const Result& rhs)
{
    *d = *rhs.d;
    return *this;
}
