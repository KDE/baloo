/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#ifndef BALOO_SEARCHSTORE_H
#define BALOO_SEARCHSTORE_H

#include <QString>
#include <QDateTime>
#include <QHash>
#include "term.h"

namespace Baloo {

class Term;
class Database;
class Transaction;
class EngineQuery;
class PostingIterator;

class SearchStore
{
public:
    SearchStore();
    ~SearchStore();

    QStringList exec(const Term& term, int offset, int limit, bool sortResults);

private:
    QByteArray fetchPrefix(const QByteArray& property) const;

    Database* m_db;
    QHash<QByteArray, QByteArray> m_prefixes;

    PostingIterator* constructQuery(Transaction* tr, const Term& term);

    EngineQuery constructContainsQuery(const QByteArray& prefix, const QString& value);
    EngineQuery constructEqualsQuery(const QByteArray& prefix, const QString& value);
    EngineQuery constructTypeQuery(const QString& type);

    PostingIterator* constructRatingQuery(Transaction* tr, int rating);
    PostingIterator* constructMTimeQuery(Transaction* tr, const QDateTime& dt, Term::Comparator com);
};

}

#endif // BALOO_SEARCHSTORE_H
