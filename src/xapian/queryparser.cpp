/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "queryparser.h"

using namespace Baloo;

QueryParser::QueryParser()
    : m_db(0)
{
}

void QueryParser::setDatabase(Xapian::Database* db)
{
    m_db = db;
}

Xapian::Query QueryParser::parseQuery(const QString& str)
{
    Xapian::QueryParser parser;
    parser.set_default_op(Xapian::Query::OP_AND);

    if (m_db)
        parser.set_database(*m_db);

    int flags = Xapian::QueryParser::FLAG_PHRASE | Xapian::QueryParser::FLAG_PARTIAL;

    std::string stdString(str.toUtf8().constData());
    return parser.parse_query(stdString, flags);
}
