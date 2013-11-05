/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "contactquery.h"
#include "resultiterator_p.h"
#include "xapian.h"

#include <QList>
#include <KDebug>
#include <KStandardDirs>

using namespace Baloo;

class ContactQuery::Private {
public:
    QString name;
    QString nick;
    QString email;
    QString uid;
    QString any;

    int limit;
    MatchCriteria criteria;
};

ContactQuery::ContactQuery()
    : Query()
    , d(new Private)
{
}

ContactQuery::~ContactQuery()
{
    delete d;
}

void ContactQuery::matchName(const QString& name)
{
    d->name = name;
}

void ContactQuery::matchNickname(const QString& nick)
{
    d->nick = nick;
}

void ContactQuery::matchEmail(const QString& email)
{
    d->email = email;
}

void ContactQuery::matchUID(const QString& uid)
{
    d->uid = uid;
}

void ContactQuery::match(const QString& str)
{
    d->any = str;
}

int ContactQuery::limit() const
{
    return d->limit;
}

void ContactQuery::setLimit(int limit)
{
    d->limit = limit;
}

ContactQuery::MatchCriteria ContactQuery::matchCriteria() const
{
    return d->criteria;
}

void ContactQuery::setMatchCriteria(ContactQuery::MatchCriteria m)
{
    d->criteria = m;
}

ResultIterator ContactQuery::exec()
{
    QString dir = KStandardDirs::locateLocal("data", "baloo/email/");
    Xapian::Database db(dir.toStdString());

    QList<Xapian::Query> m_queries;

    if (d->criteria == ExactMatch) {
        if (d->any.size()) {
            m_queries << Xapian::Query(d->any.toStdString());
        }

        if (d->name.size()) {
            m_queries << Xapian::Query("NA" + d->name.toStdString());
        }

        if (d->nick.size()) {
            m_queries << Xapian::Query("NI" + d->nick.toStdString());
        }

        if (d->email.size()) {
            m_queries << Xapian::Query(d->email.toStdString());
        }

        if (d->uid.size()) {
            m_queries << Xapian::Query(d->uid.toStdString());
        }
    }
    else if (d->criteria == StartsWithMatch) {
        if (d->any.size()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            m_queries << parser.parse_query(d->any.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (d->name.size()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NA");

            m_queries << parser.parse_query(d->name.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (d->nick.size()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NI");

            m_queries << parser.parse_query(d->nick.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        // FIXME: Check for exact match?
        if (d->email.size()) {
            Xapian::QueryParser parser;
            parser.set_database(db);

            m_queries << parser.parse_query(d->email.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (d->uid.size()) {
            Xapian::QueryParser parser;
            parser.set_database(db);

            m_queries << parser.parse_query(d->uid.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    Xapian::Query query(Xapian::Query::OP_OR, m_queries.begin(), m_queries.end());
    kDebug() << query.get_description().c_str();

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    if (d->limit == 0)
        d->limit = 10000;

    Xapian::MSet matches = enquire.get_mset(0, d->limit);

    ResultIterator iter;
    iter.d->init(matches);
    return iter;
}
