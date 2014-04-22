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

#include "contactcompleter.h"
#include <xapian.h>

#include <KStandardDirs>
#include <KDebug>

#include <QFile>

using namespace Baloo::PIM;

ContactCompleter::ContactCompleter(const QString& prefix, int limit)
    : m_prefix(prefix.toLower())
    , m_limit(limit)
{

}

QStringList ContactCompleter::complete()
{
    const QString dir = KGlobal::dirs()->localxdgdatadir() + "baloo/emailContacts/";
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dir).constData());
    }
    catch (const Xapian::DatabaseError& e) {
        kWarning() << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        return QStringList();
    }

    Xapian::QueryParser parser;
    parser.set_database(db);

    std::string prefix(m_prefix.toUtf8().constData());
    int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
    Xapian::Query q = parser.parse_query(prefix, flags);

    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset = enq.get_mset(0, m_limit);
    Xapian::MSetIterator mit = mset.begin();

    QStringList list;
    Xapian::MSetIterator end = mset.end();
    for (; mit != end; ++mit) {
        std::string str = mit.get_document().get_data();
        const QString entry = QString::fromUtf8(str.c_str(), str.length());
        list << entry;
    }

    return list;
}
