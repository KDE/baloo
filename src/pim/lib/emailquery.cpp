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

#include "emailquery.h"
#include "resultiterator_p.h"
#include "xapian.h"
#include "../search/email/agepostingsource.h"

#include <KStandardDirs>

using namespace Baloo::PIM;

class EmailQuery::Private
{
  public:
    Private();

    QStringList involves;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    QString from;

    QList<Akonadi::Collection::Id> collections;

    char important;
    char read;
    char attachment;

    QString matchString;
    QString subjectMatchString;
    QString bodyMatchString;

    int limit;
};

EmailQuery::Private::Private():
    important('0'),
    read('0'),
    attachment('0'),
    limit(0)
{
}

EmailQuery::EmailQuery():
    Query(),
    d(new Private)
{
}

EmailQuery::~EmailQuery()
{
    delete d;
}

void EmailQuery::addInvolves(const QString& email)
{
    d->involves << email;
}

void EmailQuery::setInvolves(const QStringList& involves)
{
    d->involves = involves;
}

void EmailQuery::addBcc(const QString& bcc)
{
    d->bcc << bcc;
}

void EmailQuery::setBcc(const QStringList& bcc)
{
    d->bcc = bcc;
}

void EmailQuery::setCc(const QStringList& cc)
{
    d->cc = cc;
}

void EmailQuery::setFrom(const QString& from)
{
    d->from = from;
}

void EmailQuery::addTo(const QString& to)
{
    d->to << to;
}

void EmailQuery::setTo(const QStringList& to)
{
    d->to = to;
}

void EmailQuery::addCc(const QString& cc)
{
    d->cc << cc;
}

void EmailQuery::addFrom(const QString& from)
{
    d->from = from;
}

void EmailQuery::addCollection(Akonadi::Collection::Id id)
{
    d->collections << id;
}

void EmailQuery::setCollection(const QList< Akonadi::Entity::Id >& collections)
{
    d->collections = collections;
}

int EmailQuery::limit() const
{
    return d->limit;
}

void EmailQuery::setLimit(int limit)
{
    d->limit = limit;
}

void EmailQuery::matches(const QString& match)
{
    d->matchString = match;
}

void EmailQuery::subjectMatches(const QString& subjectMatch)
{
    d->subjectMatchString = subjectMatch;
}

void EmailQuery::bodyMatches(const QString &bodyMatch)
{
    d->bodyMatchString =  bodyMatch;
}

void EmailQuery::setAttachment(bool hasAttachment)
{
    d->attachment = hasAttachment ? 'T' : 'F';
}

void EmailQuery::setImportant(bool important)
{
    d->important = important ? 'T' : 'F';
}

void EmailQuery::setRead(bool read)
{
    d->read = read ? 'T' : 'F';
}

ResultIterator EmailQuery::exec()
{
    const QString dir = KStandardDirs::locateLocal("data", "baloo/email/");
    Xapian::Database db(dir.toUtf8().constData());

    QList<Xapian::Query> m_queries;

    if (!d->involves.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");
        parser.add_prefix("", "T");
        parser.add_prefix("", "CC");
        parser.add_prefix("", "BCC");

        // vHanda: Do we really need the query parser over here?
        Q_FOREACH (const QString& str, d->involves) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->from.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");

        m_queries << parser.parse_query(d->from.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->to.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "T");

        Q_FOREACH (const QString& str, d->to) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->cc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "CC");

        Q_FOREACH (const QString& str, d->cc) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->bcc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BC");

        Q_FOREACH (const QString& str, d->bcc) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->subjectMatchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "S");
        parser.set_default_op(Xapian::Query::OP_AND);

        m_queries << parser.parse_query(d->subjectMatchString.toStdString(),
                                        Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->collections.isEmpty()) {
        Xapian::Query query;
        Q_FOREACH (const Akonadi::Collection::Id& id, d->collections) {
            QString c = QString::number(id);
            Xapian::Query q = Xapian::Query('C' + c.toStdString());

            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
    }

    if (!d->bodyMatchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BO");

        m_queries << parser.parse_query(d->bodyMatchString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (d->important == 'T')
        m_queries << Xapian::Query("BI");
    else if (d->important == 'F')
        m_queries << Xapian::Query("BNI");

    if (d->read == 'T')
        m_queries << Xapian::Query("BR");
    else if (d->read == 'F')
        m_queries << Xapian::Query("BNR");

    if (d->attachment == 'T')
        m_queries << Xapian::Query("BA");
    else if (d->attachment == 'F')
        m_queries << Xapian::Query("BNA");

    if (!d->matchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.set_default_op(Xapian::Query::OP_AND);

        const QStringList list = d->matchString.split(QRegExp("\\s"), QString::SkipEmptyParts);
        Q_FOREACH (const QString& s, list) {
            m_queries << parser.parse_query(s.toStdString(),
                                            Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    Xapian::Query query(Xapian::Query::OP_AND, m_queries.begin(), m_queries.end());

    AgePostingSource ps(0);
    query = Xapian::Query(Xapian::Query::OP_AND_MAYBE, query, Xapian::Query(&ps));

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    if (d->limit == 0)
        d->limit = 1000000;

    Xapian::MSet mset = enquire.get_mset(0, d->limit);

    ResultIterator iter;
    iter.d->init(mset);
    return iter;
}
