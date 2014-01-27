/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include "searchplugin.h"

#include <query.h>
#include <term.h>
#include <baloo/pim/emailquery.h>

#include <akonadi/searchquery.h>

#include <KDebug>
#include <QtPlugin>

static Baloo::Term::Operation mapRelation(Akonadi::SearchTerm::Relation relation) {
    if (relation == Akonadi::SearchTerm::RelAnd){
        return Baloo::Term::And;
    }
    return Baloo::Term::Or;
}

Baloo::Query SearchPlugin::fromAkonadiQuery(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes)
{
    const Akonadi::SearchQuery searchQuery = Akonadi::SearchQuery::fromJSON(akonadiQuery.toLatin1());
    if (searchQuery.isNull()) {
        kWarning() << "invalid query " << akonadiQuery;
        return Baloo::Query();
    }
    const Akonadi::SearchTerm term = searchQuery.term();

    Baloo::Query query;
    if (term.subTerms().isEmpty()) {
        query.setSearchString(term.value().toString());
        return query;
    } else {
        Baloo::Term t(mapRelation(term.relation()));
        //TODO use mimetypes instead?
        if (term.key() == "type") {
            if (term.value().toString() == "email") {
                Baloo::PIM::EmailQuery emailQuery;
                emailQuery.setCollection(collections);

                Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
                    switch (Akonadi::EmailSearchTerm::fromKey(subterm.key())) {
                        case Akonadi::EmailSearchTerm::Subject:
                            emailQuery.subjectMatches(subterm.value().toString());
                            break;
                        case Akonadi::EmailSearchTerm::Headers:
                            break;
                        case Akonadi::EmailSearchTerm::Age:
                            break;
                        case Akonadi::EmailSearchTerm::All:
                            emailQuery.matches(subterm.value().toString());
                        case Akonadi::EmailSearchTerm::Body:
                            break;
                        case Akonadi::EmailSearchTerm::CC:
                            emailQuery.addCc(subterm.value().toString());
                            break;
                    }
                }
                return emailQuery;
            }
        }
    }
    return Baloo::Query();
}



QSet<qint64> SearchPlugin::search(const QString &queryString, const QList<qint64> &collections, const QStringList &mimeTypes)
{
    QSet<qint64> resultSet;

    Baloo::Query query = fromAkonadiQuery(queryString, collections, mimeTypes);
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QByteArray id = iter.id();
        const int fid = Baloo::deserialize("akonadi", id);
        resultSet << fid;
    }
    return resultSet;
}

Q_EXPORT_PLUGIN2(akonadi_baloo_searchplugin, SearchPlugin)
