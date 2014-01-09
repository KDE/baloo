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

#include "util.h"
#include <xapian.h>

#include <QString>

void Baloo::updateIndexingLevel(Xapian::Document& doc, int level)
{
    Xapian::TermIterator it = doc.termlist_begin();
    it.skip_to("Z");

    if (it != doc.termlist_end()) {
        std::string term = *it;
        if (term.length() && term[0] == 'Z') {
            doc.remove_term(term);
        }
    }

    const QString term = QLatin1Char('Z') + QString::number(level);
    doc.add_term(term.toStdString());
}

void Baloo::reopenIfRequired(Xapian::Database* db)
{
    try {
        db->get_doccount();
    }
    catch (const Xapian::DatabaseModifiedError&) {
        db->reopen();
    }
}
