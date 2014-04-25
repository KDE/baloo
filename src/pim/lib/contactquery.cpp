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

#include "contactquery.h"
#include "resultiterator_p.h"
#include "xapian.h"

#include <QDebug>

#include <QList>
#include <QDebug>
#include <QStandardPaths>

#include <QFile>

using namespace Baloo::PIM;

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
    d->criteria = StartsWithMatch;
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
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/baloo/contacts/";
    Xapian::Database db;

    try {
        db = Xapian::Database(QFile::encodeName(dir).constData());
    } catch (const Xapian::DatabaseError& e) {
        qWarning() << "Failed to open Xapian database:" << QString::fromStdString(e.get_error_string());
        return ResultIterator();
    }

    QList<Xapian::Query> m_queries;

    if (d->criteria == ExactMatch) {
        if (!d->any.isEmpty()) {
            const QByteArray ba = d->any.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->name.isEmpty()) {
            const QByteArray ba = "NA" + d->name.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->nick.isEmpty()) {
            const QByteArray ba = "NI" + d->nick.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->email.isEmpty()) {
            const QByteArray ba = d->email.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->uid.isEmpty()) {
            m_queries << Xapian::Query(d->uid.toStdString());
        }
    }
    else if (d->criteria == StartsWithMatch) {
        if (!d->any.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            const QByteArray ba = d->any.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->name.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NA");
            const QByteArray ba = d->name.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->nick.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NI");
            const QByteArray ba = d->nick.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        // FIXME: Check for exact match?
        if (!d->email.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            const QByteArray ba = d->email.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->uid.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);

            m_queries << parser.parse_query(d->uid.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    Xapian::Query query(Xapian::Query::OP_OR, m_queries.begin(), m_queries.end());
    qDebug() << query.get_description().c_str();

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    if (d->limit == 0)
        d->limit = 10000;

    Xapian::MSet matches = enquire.get_mset(0, d->limit);

    ResultIterator iter;
    iter.d->init(matches);
    return iter;
}
