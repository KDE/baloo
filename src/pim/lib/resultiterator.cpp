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

#include "resultiterator_p.h"

using namespace Baloo::PIM;

ResultIterator::ResultIterator()
    : d(new Private)
{
    d->m_firstElement = false;
}

ResultIterator::ResultIterator(const ResultIterator& ri)
    : d(new Private(*ri.d))
{
}

ResultIterator::~ResultIterator()
{
    delete d;
}

bool ResultIterator::next()
{
    if (d->m_iter == d->m_end)
        return false;

    if (d->m_firstElement) {
        d->m_iter = d->m_mset.begin();
        d->m_firstElement = false;
        return (d->m_iter != d->m_end);
    }

    d->m_iter++;
    return (d->m_iter != d->m_end);
}

Akonadi::Entity::Id ResultIterator::id()
{
    //qDebug() << d->m_iter.get_rank() << d->m_iter.get_weight();
    return *(d->m_iter);
}
