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

#include "queryrunnable.h"
#include <QAtomicInt>

using namespace Baloo;

class QueryRunnable::Private {
public:
    Query m_query;
    QAtomicInt m_stop;

    bool stopRequested() const {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        return m_stop.load();
#else
        return m_stop.loadRelaxed();
#endif
    }

};

QueryRunnable::QueryRunnable(const Query& query, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->m_query = query;
    d->m_stop = false;
}

QueryRunnable::~QueryRunnable()
{
    delete d;
}

void QueryRunnable::stop()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    d->m_stop.store(true);
#else
    d->m_stop.storeRelaxed(true);
#endif
}

void QueryRunnable::run()
{
    ResultIterator it = d->m_query.exec();
    while (!d->stopRequested() && it.next()) {
        Q_EMIT queryResult(this, it.filePath());
    }

    Q_EMIT finished(this);
}

