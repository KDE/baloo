/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014 Laurent Montel <montel@kde.org>
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

#include "notequery.h"
#include "resultiterator_p.h"
#include "xapian.h"

#include <QList>
#include <KDebug>
#include <KStandardDirs>

using namespace Baloo::PIM;

class NoteQuery::Private {
public:
    Private()
        : limit(0)
    {

    }

    QString title;
    QString note;
    int limit;
};

NoteQuery::NoteQuery()
    : Query()
    , d(new Private)
{
}

NoteQuery::~NoteQuery()
{
    delete d;
}

void NoteQuery::matchTitle(const QString &title)
{
    d->title = title;
}

void NoteQuery::matchNote(const QString &note)
{
    d->note = note;
}

void NoteQuery::setLimit(int limit)
{
    d->limit = limit;
}

int NoteQuery::limit() const
{
    return d->limit;
}

ResultIterator NoteQuery::exec()
{
    QString dir = KStandardDirs::locateLocal("data", "baloo/notes/");
    Xapian::Database db(dir.toUtf8().constData());

    QList<Xapian::Query> m_queries;

    if (!d->note.isEmpty()) {
        //TODO
        /*
        Xapian::QueryParser parser;
        parser.set_database(db);
        m_queries << parser.parse_query(d->note.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
        */
    }

    if (!d->title.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "S");
        parser.set_default_op(Xapian::Query::OP_AND);

        m_queries << parser.parse_query(d->title.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
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
