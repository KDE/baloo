/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BALOO_POSTINGITERATOR_H
#define BALOO_POSTINGITERATOR_H

#include <QVector>
#include "engine_export.h"

namespace Baloo {

/**
 * A PostingIterator is an abstract base class which can be used to iterate
 * over all the "postings" or "documents" which are particular term appears.
 *
 * All PostingIterators should iterate over a list of non-decreasing document ids.
 */
class BALOO_ENGINE_EXPORT PostingIterator
{
public:
    virtual ~PostingIterator();

    virtual quint64 next() = 0;
    virtual quint64 docId() const = 0;
    virtual quint64 skipTo(quint64 docId);

    virtual QVector<uint> positions();
};
}

#endif // BALOO_POSTINGITERATOR_H
