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
#include "searchstore.h"
#include "result.h"
#include <KDebug>

using namespace Baloo;

ResultIteratorPrivate::ResultIteratorPrivate()
    : queryId(0)
    , store(0)
{
}

/*
 * FIXME: The linker complains if it is not present in the header.
ResultIteratorPrivate::~ResultIteratorPrivate()
{
    if (store) {
        store->close(queryId);
    }
}
*/


ResultIterator::ResultIterator(int id, SearchStore* store)
    : d(new ResultIteratorPrivate)
{
    d->queryId = id;
    d->store = store;
}

ResultIterator::ResultIterator()
    : d(new ResultIteratorPrivate)
{
}

bool ResultIterator::next()
{
    if (d->store)
        return d->store->next(d->queryId);
    else
        return false;
}

Item::Id ResultIterator::id() const
{
    if (d->store)
        return d->store->id(d->queryId);
    else
        return Item::Id();
}

QUrl ResultIterator::url() const
{
    if (d->store)
        return d->store->url(d->queryId);
    else
        return QUrl();
}

QString ResultIterator::text() const
{
    if (d->store)
        return d->store->text(d->queryId);
    else
        return QString();
}

QString ResultIterator::icon() const
{
    if (d->store)
        return d->store->icon(d->queryId);
    else
        return QString();
}

Result ResultIterator::result() const
{
    Result res;
    res.setId(id());
    res.setText(text());
    res.setIcon(icon());
    res.setUrl(url());

    return res;
}
