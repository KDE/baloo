/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "queryrunnable.h"
#include <QAtomicInt>

using namespace Baloo;

class QueryRunnable::Private {
public:
    Query m_query;
    QAtomicInt m_stop;

    bool stopRequested() const {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
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

