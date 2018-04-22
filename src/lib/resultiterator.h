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

#ifndef BALOO_CORE_RESULT_ITERATOR_H
#define BALOO_CORE_RESULT_ITERATOR_H

#include "core_export.h"

#include <QString>

namespace Baloo {

class SearchStore;
class Result;
class ResultIteratorPrivate;

/**
 * @class ResultIterator resultiterator.h <Baloo/ResultIterator>
 */
class BALOO_CORE_EXPORT ResultIterator
{
public:
    ResultIterator(const ResultIterator& rhs);
    ~ResultIterator();

    bool next();
    QString filePath() const;

private:
    ResultIterator(const QStringList& results);
    ResultIteratorPrivate* d;

    friend class Query;
};

}
#endif // BALOO_CORE_RESULT_ITERATOR_H
