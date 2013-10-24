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

#include "queryiterator.h"
#include "query.h"

Query::Query()
{
    m_path = QLatin1String("/tmp/xap/");

    m_important = false;
    m_attachment = false;
    m_read = false;
}

void Query::addInvolves(const QString& email)
{
    m_involves << email;
}

void Query::setInvolves(const QStringList& involves)
{
    m_involves = involves;
}

void Query::addBcc(const QString& bcc)
{
    m_bcc << bcc;
}

void Query::setBcc(const QStringList& bcc)
{
    m_bcc = bcc;
}

void Query::setCc(const QStringList& cc)
{
    m_cc = cc;
}

void Query::setFrom(const QString& from)
{
    m_from = from;
}

void Query::addTo(const QString& to)
{
    m_to << to;
}

void Query::setTo(const QStringList& to)
{
    m_to = to;
}

void Query::addCc(const QString& cc)
{
    m_cc << cc;
}

void Query::addFrom(const QString& from)
{
    m_from = from;
}

void Query::addCollection(Akonadi::Collection::Id id)
{
    m_collections << id;
}

void Query::setCollection(const QList< Akonadi::Entity::Id >& collections)
{
    m_collections = collections;
}

int Query::limit()
{
    return m_limit;
}

void Query::setLimit(int limit)
{
    m_limit = limit;
}

void Query::matches(const QString& match)
{
    m_matchString = match;
}

void Query::bodyMatches(const QString& match)
{
    m_bodyMatchString = match;
}

void Query::subjectMatches(const QString& subjectMatch)
{
    m_subjectMatchString = subjectMatch;
}

void Query::setAttachment(bool hasAttachment)
{
    m_attachment = hasAttachment;
}

void Query::setImportant(bool important)
{
    m_important = important;
}

void Query::setRead(bool read)
{
    m_read = read;
}

QueryIterator Query::exec()
{
    Xapian::Database databases;
    QList<Xapian::Query> m_queries;

    if (m_involves.size()) {
        Xapian::Query query;
        Q_FOREACH (const QString& email, m_involves) {
            Xapian::Query q = Xapian::Query(email.toStdString());
            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
        databases.add_database(Xapian::Database((m_path + "from").toStdString()));
        databases.add_database(Xapian::Database((m_path + "to").toStdString()));
        databases.add_database(Xapian::Database((m_path + "cc").toStdString()));
        databases.add_database(Xapian::Database((m_path + "bcc").toStdString()));
    }

    if (m_from.size()) {
        m_queries << Xapian::Query(m_from.toStdString());
        databases.add_database(Xapian::Database((m_path + "from").toStdString()));
    }

    if (m_to.size()) {
        Xapian::Query query;
        Q_FOREACH (const QString& to, m_to) {
            Xapian::Query q = Xapian::Query(to.toStdString());
            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
        databases.add_database(Xapian::Database((m_path + "to").toStdString()));
    }

    if (m_cc.size()) {
        Xapian::Query query;
        Q_FOREACH (const QString& cc, m_cc) {
            Xapian::Query q = Xapian::Query(cc.toStdString());
            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
        databases.add_database(Xapian::Database((m_path + "cc").toStdString()));
    }

    if (m_bcc.size()) {
        Xapian::Query query;
        Q_FOREACH (const QString& bcc, m_bcc) {
            Xapian::Query q = Xapian::Query(bcc.toStdString());
            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
        databases.add_database(Xapian::Database((m_path + "bcc").toStdString()));
    }

    if (m_subjectMatchString.size()) {
        Xapian::Database db((m_path + "subject").toStdString());

        Xapian::QueryParser parser;
        parser.set_database(db);
        Xapian::Query q = parser.parse_query(m_subjectMatchString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);

        m_queries << q;
        databases.add_database(db);
    }

    if (m_bodyMatchString.size()) {
        Xapian::Database db((m_path + "body").toStdString());

        Xapian::QueryParser parser;
        parser.set_database(db);
        Xapian::Query q = parser.parse_query(m_bodyMatchString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);

        m_queries << q;
        databases.add_database(db);
    }

    bool allDbNeeded = false;

    if (m_collections.size()) {
        Xapian::Query query;
        Q_FOREACH (const Akonadi::Collection::Id& id, m_collections) {
            QString collectionStr = QLatin1String("XC") + QString::number(id);
            Xapian::Query q = Xapian::Query(collectionStr.toStdString());

            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
        allDbNeeded = true;
    }

    if (m_important) {
        m_queries << Xapian::Query("XisImportant");
        allDbNeeded = true;
    }

    if (m_read) {
        m_queries << Xapian::Query("XisRead");
        allDbNeeded = true;
    }

    if (m_attachment) {
        m_queries << Xapian::Query("XisAttachment");
        allDbNeeded = true;
    }

    if (m_matchString.size()) {
        Xapian::Database db((m_path + "all").toStdString());

        Xapian::QueryParser parser;
        parser.set_database(db);
        Xapian::Query q = parser.parse_query(m_matchString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);

        m_queries << q;
        allDbNeeded = true;
    }

    if (allDbNeeded)
        databases.add_database(Xapian::Database((m_path + "all").toStdString()));

    Xapian::Query query(Xapian::Query::OP_AND, m_queries.begin(), m_queries.end());
    kDebug() << query.get_description().c_str();

    Xapian::Enquire enquire(databases);
    enquire.set_query(query);

    if (m_limit == 0)
        m_limit = 1000000;

    Xapian::MSet matches = enquire.get_mset(0, m_limit);
    return QueryIterator(matches);
}
