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

#ifndef BALOO_QUERYPARSER_H
#define BALOO_QUERYPARSER_H

#include <QString>
#include <xapian.h>
#include "xapian_export.h"

namespace Baloo {

class BALOO_XAPIAN_EXPORT XapianQueryParser
{
public:
    XapianQueryParser();

    void setDatabase(Xapian::Database* db);
    Xapian::Query parseQuery(const QString& str, const QString& prefix = QString());

    /**
     * Expands word to every possible option which it can be expanded to.
     */
    Xapian::Query expandWord(const QString& word, const QString& prefix = QString());

    /**
     * Set if each word in the string should be treated as a partial word
     * and should be expanded to every possible word.
     */
    void setAutoExapand(bool autoexpand);

private:
    Xapian::Database* m_db;
    bool m_autoExpand;
};

}

#endif // BALOO_QUERYPARSER_H
