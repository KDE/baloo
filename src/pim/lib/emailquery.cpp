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

EmailQuery::EmailQuery()
{
    m_path = QLatin1String("/tmp/xap/");

    m_important = 'O';
    m_attachment = 'O';
    m_read = 'O';
}

void EmailQuery::addInvolves(const QString& email)
{
    m_involves << email;
}

void EmailQuery::setInvolves(const QStringList& involves)
{
    m_involves = involves;
}

void EmailQuery::addBcc(const QString& bcc)
{
    m_bcc << bcc;
}

void EmailQuery::setBcc(const QStringList& bcc)
{
    m_bcc = bcc;
}

void EmailQuery::setCc(const QStringList& cc)
{
    m_cc = cc;
}

void EmailQuery::setFrom(const QString& from)
{
    m_from = from;
}

void EmailQuery::addTo(const QString& to)
{
    m_to << to;
}

void EmailQuery::setTo(const QStringList& to)
{
    m_to = to;
}

void EmailQuery::addCc(const QString& cc)
{
    m_cc << cc;
}

void EmailQuery::addFrom(const QString& from)
{
    m_from = from;
}

void EmailQuery::addCollection(Akonadi::Collection::Id id)
{
    m_collections << id;
}

void EmailQuery::setCollection(const QList< Akonadi::Entity::Id >& collections)
{
    m_collections = collections;
}

int EmailQuery::limit()
{
    return m_limit;
}

void EmailQuery::setLimit(int limit)
{
    m_limit = limit;
}

void EmailQuery::matches(const QString& match)
{
    m_matchString = match;
}

void EmailQuery::subjectMatches(const QString& subjectMatch)
{
    m_subjectMatchString = subjectMatch;
}

void EmailQuery::setAttachment(bool hasAttachment)
{
    m_attachment = hasAttachment ? 'T' : 'F';
}

void EmailQuery::setImportant(bool important)
{
    m_important = important ? 'T' : 'F';
}

void EmailQuery::setRead(bool read)
{
    m_read = read ? 'T' : 'F';
}

ResultIterator EmailQuery::exec()
{
    QString dir = KStandardDirs::locateLocal("data", "baloo/email/");
    Xapian::Database db(dir.toUtf8().constData());

    QList<Xapian::Query> m_queries;

    if (!m_involves.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");
        parser.add_prefix("", "T");
        parser.add_prefix("", "CC");
        parser.add_prefix("", "BCC");

        // vHanda: Do we really need the query parser over here?
        Q_FOREACH (const QString& str, m_involves) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!m_from.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");

        m_queries << parser.parse_query(m_from.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!m_to.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "T");

        Q_FOREACH (const QString& str, m_to) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!m_cc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "CC");

        Q_FOREACH (const QString& str, m_cc) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!m_bcc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BC");

        Q_FOREACH (const QString& str, m_bcc) {
            m_queries << parser.parse_query(str.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!m_subjectMatchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "S");
        parser.set_default_op(Xapian::Query::OP_AND);

        m_queries << parser.parse_query(m_subjectMatchString.toStdString(),
                                        Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!m_collections.isEmpty()) {
        Xapian::Query query;
        Q_FOREACH (const Akonadi::Collection::Id& id, m_collections) {
            QString c = QString::number(id);
            Xapian::Query q = Xapian::Query('C' + c.toStdString());

            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
    }

    if (m_important == 'T')
        m_queries << Xapian::Query("BI");
    else if (m_important == 'F')
        m_queries << Xapian::Query("BNI");

    if (m_read == 'T')
        m_queries << Xapian::Query("BR");
    else if (m_important == 'F')
        m_queries << Xapian::Query("BNR");

    if (m_attachment == 'T')
        m_queries << Xapian::Query("BA");
    else if (m_attachment == 'F')
        m_queries << Xapian::Query("BNA");

    if (!m_matchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.set_default_op(Xapian::Query::OP_AND);

        QStringList list = m_matchString.split(QRegExp("\\s"), QString::SkipEmptyParts);
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

    if (m_limit == 0)
        m_limit = 1000000;

    Xapian::MSet mset = enquire.get_mset(0, m_limit);

    ResultIterator iter;
    iter.d->init(mset);
    return iter;
}
