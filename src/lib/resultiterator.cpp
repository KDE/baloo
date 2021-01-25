/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resultiterator.h"
#include "result_p.h"
#include "searchstore.h"
#include <vector>

using namespace Baloo;

class Baloo::ResultIteratorPrivate {
public:
    ResultIteratorPrivate() = default;

    ~ResultIteratorPrivate() {
    }

    ResultList results;
    int pos = -1;
};

ResultIterator::ResultIterator(ResultList&& res)
    : d(new ResultIteratorPrivate)
{
    d->results = res;
}

#if BALOO_CORE_BUILD_DEPRECATED_SINCE(5, 55)
ResultIterator::ResultIterator(const ResultIterator& rhs)
    : d(new ResultIteratorPrivate)
{
    d->results = rhs.d->results;
}
#endif

ResultIterator::ResultIterator(ResultIterator &&rhs)
    : d(rhs.d)
{
    rhs.d = nullptr;
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

QString ResultIterator::filePath() const
{
    Q_ASSERT(d->pos >= 0 && d->pos < d->results.size());
    return QString::fromUtf8(d->results.at(d->pos).filePath);
}

QByteArray ResultIterator::documentId() const
{
    Q_ASSERT(d->pos >= 0 && d->pos < d->results.size());
    return QByteArray::number(d->results.at(d->pos).documentId, 16);
}
