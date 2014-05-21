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
#include <KABC/Addressee>
#include <KABC/ContactGroup>
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
    if (comparator == Akonadi::SearchTerm::CondLessOrEqual){
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

Baloo::Term recursiveEmailTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Baloo::Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Baloo::Term newTerm = recursiveEmailTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        kDebug() << term.key() << term.value();
        const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
        switch (field) {
            case Akonadi::EmailSearchTerm::Message: {
                Baloo::Term s(Baloo::Term::Or);
                s.setNegation(term.isNegated());
                s.addSubTerm(Baloo::Term("body", term.value(), mapComparator(term.condition())));
                s.addSubTerm(Baloo::Term("headers", term.value(), mapComparator(term.condition())));
                return s;
            }
            case Akonadi::EmailSearchTerm::Body:
                return getTerm(term, "body");
            case Akonadi::EmailSearchTerm::Headers:
                return getTerm(term, "headers");
            case Akonadi::EmailSearchTerm::ByteSize:
                return getTerm(term, "size");
            case Akonadi::EmailSearchTerm::HeaderDate: {
                Baloo::Term s("date", QString::number(term.value().toDateTime().toTime_t()), mapComparator(term.condition()));
                s.setNegation(term.isNegated());
                return s;
            }
            case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
                Baloo::Term s("onlydate", QString::number(term.value().toDate().toJulianDay()), mapComparator(term.condition()));
                s.setNegation(term.isNegated());
                return s;
            }
            case Akonadi::EmailSearchTerm::Subject:
                return getTerm(term, "subject");
            case Akonadi::EmailSearchTerm::HeaderFrom:
                return getTerm(term, "from");
            case Akonadi::EmailSearchTerm::HeaderTo:
                return getTerm(term, "to");
            case Akonadi::EmailSearchTerm::HeaderCC:
                return getTerm(term, "cc");
            case Akonadi::EmailSearchTerm::HeaderBCC:
                return getTerm(term, "bcc");
            case Akonadi::EmailSearchTerm::MessageStatus:
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                    return Baloo::Term("isimportant", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::ToAct)) {
                    return Baloo::Term("istoact", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Watched)) {
                    return Baloo::Term("iswatched", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
                    return Baloo::Term("isdeleted", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Spam)) {
                    return Baloo::Term("isspam", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
                    return Baloo::Term("isreplied", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ignored)) {
                    return Baloo::Term("isignored", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Forwarded)) {
                    return Baloo::Term("isforwarded", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Sent)) {
                    return Baloo::Term("issent", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Queued)) {
                    return Baloo::Term("isqueued", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ham)) {
                    return Baloo::Term("isham", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
                    return Baloo::Term("isread", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)) {
                    return Baloo::Term("hasattachment", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Encrypted)) {
                    return Baloo::Term("isencrypted", !term.isNegated());
                }
                if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasInvitation)) {
                    return Baloo::Term("hasinvitation", !term.isNegated());
                }
                break;
            case Akonadi::EmailSearchTerm::MessageTag:
                //search directly in akonadi? or index tags.
                break;
            case Akonadi::EmailSearchTerm::HeaderReplyTo:
                return getTerm(term, "replyto");
            case Akonadi::EmailSearchTerm::HeaderOrganization:
                return getTerm(term, "organization");
            case Akonadi::EmailSearchTerm::HeaderListId:
                return getTerm(term, "listid");
            case Akonadi::EmailSearchTerm::HeaderResentFrom:
                return getTerm(term, "resentfrom");
            case Akonadi::EmailSearchTerm::HeaderXLoop:
                return getTerm(term, "xloop");
            case Akonadi::EmailSearchTerm::HeaderXMailingList:
                return getTerm(term, "xmailinglist");
            case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
                return getTerm(term, "xspamflag");
            case Akonadi::EmailSearchTerm::Unknown:
            default:
                kWarning() << "unknown term " << term.key();
        }
    }
    return Baloo::Term();
}

Baloo::Term recursiveNoteTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Baloo::Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Baloo::Term newTerm = recursiveNoteTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        kDebug() << term.key() << term.value();
        const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::EmailSearchTerm::Subject:
            return getTerm(term, "subject");
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, "body");
        default:
            kWarning() << "unknown term " << term.key();
        }
    }
    return Baloo::Term();
}

Baloo::Term recursiveContactTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Baloo::Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Baloo::Term newTerm = recursiveContactTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        kDebug() << term.key() << term.value();
        const Akonadi::ContactSearchTerm::ContactSearchField field = Akonadi::ContactSearchTerm::fromKey(term.key());
        switch (field) {
            case Akonadi::ContactSearchTerm::Name:
                return getTerm(term, "name");
            case Akonadi::ContactSearchTerm::Email:
                return getTerm(term, "email");
            case Akonadi::ContactSearchTerm::Nickname:
                return getTerm(term, "nick");
            case Akonadi::ContactSearchTerm::Uid:
                return getTerm(term, "uid");
            case Akonadi::ContactSearchTerm::Unknown:
            default:
                kWarning() << "unknown term " << term.key();
        }
    }
    return Baloo::Term();
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

    Baloo::Term t;

    if (mimeTypes.contains("message/rfc822")) {
        kDebug() << "mail query";
        query.setType("Email");
        t = recursiveEmailTermMapping(term);
    } else if (mimeTypes.contains(KABC::Addressee::mimeType())) {
        query.setType("Contact");
        t = recursiveContactTermMapping(term);
    } else if (mimeTypes.contains(QLatin1String("text/x-vnd.akonadi.note"))) {
        query.setType("Note");
        t = recursiveNoteTermMapping(term);
    } else if (mimeTypes.contains(KABC::ContactGroup::mimeType())) {
        query.setType("Contact");
        t = recursiveContactTermMapping(term);
    }
    if (t.subTerms().isEmpty()) {
        kWarning() << "no terms added";
        return QSet<qint64>();
    }

    if (searchQuery.limit() > 0) {
        query.setLimit(searchQuery.limit());
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
    kDebug() << "Got" << resultSet.count() << "results";
    return resultSet;
}

Q_EXPORT_PLUGIN2(akonadi_baloo_searchplugin, SearchPlugin)
