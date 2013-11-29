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

#include "resultiterator.h"
#include "searchstore.h"
#include "result.h"
#include <KDebug>

using namespace Baloo;

class Baloo::ResultIterator::Private {
public:
    int queryId;
    SearchStore* store;
};

ResultIterator::ResultIterator(int id, SearchStore* store)
    : d(new Private)
{
    d->queryId = id;
    d->store = store;
}

ResultIterator::ResultIterator()
    : d(new Private)
{
    d->queryId = 0;
    d->store = 0;
}

ResultIterator::~ResultIterator()
{
    if (d->store)
        d->store->close(d->queryId);
    delete d;
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

    return res;
}
