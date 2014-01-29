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

#include "query.h"
#include "term.h"
#include "resultiterator.h"

#include <akonadi/searchquery.h>

#include <KDebug>
#include <Akonadi/KMime/MessageFlags>
#include <KDateTime>
#include <QtPlugin>

static Baloo::Term::Operation mapRelation(Akonadi::SearchTerm::Relation relation) {
    if (relation == Akonadi::SearchTerm::RelAnd){
        return Baloo::Term::And;
    }
    return Baloo::Term::Or;
}

static Baloo::Term::Comparator mapComparator(Akonadi::SearchTerm::Condition comparator) {
    if (comparator == Akonadi::SearchTerm::CondContains){
        return Baloo::Term::Contains;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterOrEqual){
        return Baloo::Term::GreaterEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterThan){
        return Baloo::Term::Greater;
    }
    if (comparator == Akonadi::SearchTerm::CondEqual){
        return Baloo::Term::Equal;
    }
    if (comparator == Akonadi::SearchTerm::CondLessOrEqueal){
        return Baloo::Term::LessEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondLessThan){
        return Baloo::Term::Less;
    }
    return Baloo::Term::Auto;
}

static Baloo::Term getTerm(const Akonadi::SearchTerm &term, const QString &property) {
    Baloo::Term t(property, term.value().toString(), mapComparator(term.condition()));
    t.setNegation(term.isNegated());
    return t;
}

QSet<qint64> SearchPlugin::search(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes)
{
    const Akonadi::SearchQuery searchQuery = Akonadi::SearchQuery::fromJSON(akonadiQuery.toLatin1());
    if (searchQuery.isNull()) {
        kWarning() << "invalid query " << akonadiQuery;
        return QSet<qint64>();
    }
    const Akonadi::SearchTerm term = searchQuery.term();

    Baloo::Query query;
    if (term.subTerms().isEmpty()) {
        kWarning() << "empty query";
        return QSet<qint64>();
    }

    Baloo::Term t(mapRelation(term.relation()));
    if (mimeTypes.contains("message/rfc822")) {
        kDebug() << "mail query";
        query.setType("Email");

        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            kDebug() << subterm.key() << subterm.value();
            const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(subterm.key());
            switch (field) {
                case Akonadi::EmailSearchTerm::Body:
                    //FIXME
                    //todo somehow search the body (not possible yet)
                    query.setSearchString(subterm.value().toString());
                    break;
                case Akonadi::EmailSearchTerm::Headers:
                    //FIXME
                    //search all headers
                    query.setSearchString(subterm.value().toString());
                    break;
                case Akonadi::EmailSearchTerm::Size:
                    t.addSubTerm(getTerm(subterm, "size"));
                    break;
                case Akonadi::EmailSearchTerm::Date: {
                    const KDateTime dt = KDateTime::fromString(subterm.value().toString(), KDateTime::ISODate);
                    Baloo::Term s("date", QString::number(dt.toTime_t()), mapComparator(subterm.condition()));
                    s.setNegation(subterm.isNegated());
                    t.addSubTerm(s);
                }
                    break;
                case Akonadi::EmailSearchTerm::Subject:
                    t.addSubTerm(getTerm(subterm, "subject"));
                    break;
                case Akonadi::EmailSearchTerm::From:
                    t.addSubTerm(getTerm(subterm, "from"));
                    break;
                case Akonadi::EmailSearchTerm::To:
                    t.addSubTerm(getTerm(subterm, "to"));
                    break;
                case Akonadi::EmailSearchTerm::CC:
                    t.addSubTerm(getTerm(subterm, "cc"));
                    break;
                case Akonadi::EmailSearchTerm::BCC:
                    t.addSubTerm(getTerm(subterm, "bcc"));
                    break;
                case Akonadi::EmailSearchTerm::MessageStatus:
                    if (subterm.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                        t.addSubTerm(Baloo::Term("isimportant", !subterm.isNegated()));
                    }
                    //TODO remaining flags
                    break;
                case Akonadi::EmailSearchTerm::MessageTag:
                    //search directly in akonadi? or index tags.
                    break;
                case Akonadi::EmailSearchTerm::ReplyTo:
                    t.addSubTerm(getTerm(subterm, "replyto"));
                    break;
                case Akonadi::EmailSearchTerm::Organization:
                    t.addSubTerm(getTerm(subterm, "organization"));
                    break;
                case Akonadi::EmailSearchTerm::ListId:
//                     t.addSubTerm(getTerm(subterm, "listid"));
                    break;
                case Akonadi::EmailSearchTerm::ResentFrom:
//                     t.addSubTerm(getTerm(subterm, "resentfrom"));
                    break;
                case Akonadi::EmailSearchTerm::XLoop:
                    break;
                case Akonadi::EmailSearchTerm::XMailingList:
                    break;
                case Akonadi::EmailSearchTerm::XSpamFlag:
                    break;
                case Akonadi::EmailSearchTerm::All:
                    query.setSearchString(subterm.value().toString());
                    break;
                case Akonadi::EmailSearchTerm::Unknown:
                default:
                    kWarning() << "unknown term " << subterm.key();
            }
        }
    } else if (mimeTypes.contains("text/directory")) {
        query.setType("Contact");
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            kDebug() << subterm.key() << subterm.value();
            const Akonadi::ContactSearchTerm::ContactSearchField field = Akonadi::ContactSearchTerm::fromKey(subterm.key());
            switch (field) {
                case Akonadi::ContactSearchTerm::Name:
                    t.addSubTerm(getTerm(subterm, "name"));
                    break;
                case Akonadi::ContactSearchTerm::Email:
                    t.addSubTerm(getTerm(subterm, "email"));
                    break;
                case Akonadi::ContactSearchTerm::Nickname:
                    t.addSubTerm(getTerm(subterm, "nick"));
                    break;
                case Akonadi::ContactSearchTerm::Uid:
                    t.addSubTerm(getTerm(subterm, "uid"));
                    break;
                case Akonadi::ContactSearchTerm::Unknown:
                default:
                    kWarning() << "unknown term " << subterm.key();
            }
        }
    } else if (mimeTypes.contains("...")) {
        query.setType("ContactGroups");
        //TODO contactgroup queries
    }
    if (t.subTerms().isEmpty()) {
        kWarning() << "no terms added";
        return QSet<qint64>();
    }

    //Filter by collection if not empty
    if (!collections.isEmpty()) {
        Baloo::Term parentTerm(Baloo::Term::And);
        Baloo::Term collectionTerm(Baloo::Term::Or);
        Q_FOREACH (const qint64 col, collections) {
            collectionTerm.addSubTerm(Baloo::Term("collection", QString::number(col)));
        }
        parentTerm.addSubTerm(collectionTerm);
        parentTerm.addSubTerm(t);

        query.setTerm(parentTerm);
    } else {
        query.setTerm(t);
    }

    QSet<qint64> resultSet;
    kDebug() << query.toJSON();
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QByteArray id = iter.id();
        const int fid = Baloo::deserialize("akonadi", id);
        resultSet << fid;
    }
    return resultSet;
}

Q_EXPORT_PLUGIN2(akonadi_baloo_searchplugin, SearchPlugin)
