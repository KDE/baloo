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

#include "resultiterator.h"
#include "enginesearchstore.h"
#include "result.h"

#include <QVector>

using namespace Baloo;

class Baloo::ResultIteratorPrivate {
public:
    ResultIteratorPrivate()
        : store(0) {
    }

    ~ResultIteratorPrivate() {
    }

    QVector<uint> results;
    int pos;
    EngineSearchStore* store;
};

ResultIterator::ResultIterator(const QVector<uint>& vec, EngineSearchStore* store)
    : d(new ResultIteratorPrivate)
{
    Q_ASSERT(store);

    d->results = vec;
    d->store = store;
    d->pos = -1;
}

ResultIterator::ResultIterator(const ResultIterator& rhs)
    : d(rhs.d)
{
}

ResultIterator::~ResultIterator()
{
    delete d;
}

bool ResultIterator::next()
{
    d->pos++;
    return d->pos < d->results.size();
}

uint ResultIterator::id() const
{
    Q_ASSERT(d->pos >= 0 && d->pos < d->results.size());
    return d->results.at(d->pos);
}

QString ResultIterator::filePath() const
{
    return d->store->filePath(id());
}

Result ResultIterator::result() const
{
    Result res;
    res.setId(id());
    res.setFilePath(filePath());

    return res;
}
