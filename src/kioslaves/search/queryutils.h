/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QUERYUTILS_H
#define QUERYUTILS_H

#include "kext.h"

#include <KUrl>
#include <KDebug>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/OptionalTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NFO>

using namespace Nepomuk2::Vocabulary;

namespace Nepomuk2
{
namespace Query
{
/**
 * KIO specific query handling shared by the KIO slave and the kded search module
 */
bool parseQueryUrl(const KUrl& url, Query& query, QString& sparqlQuery)
{
    // parse URL (this may fail in which case we fall back to pure SPARQL below)
    query = Nepomuk2::Query::Query::fromQueryUrl(url);

    if (query.isValid()) {
        QList<Query::RequestProperty> reqProperties;

        reqProperties << Query::RequestProperty(NIE::url(), false);
        query.setRequestProperties(reqProperties);
    } else {
        // the URL contains pure sparql.
        sparqlQuery = Nepomuk2::Query::Query::sparqlFromQueryUrl(url);
        kDebug() << "Extracted SPARL query" << sparqlQuery;
    }

    return query.isValid() || !sparqlQuery.isEmpty();
}
}
}

#endif // QUERYUTILS_H
