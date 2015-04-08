/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014  Vishesh Handa <me@vhanda.in>
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

int XapianSearchStore::exec(const Query& query)
{
    if (!m_db)
        return 0;

    while (1) {
        try {
            QMutexLocker lock(&m_mutex);
            try {
                m_db->reopen();
            } catch (Xapian::DatabaseError& e) {
                qDebug() << "Failed to reopen database" << dbPath() << ":" <<  QString::fromStdString(e.get_msg());
                return 0;
            }

            Xapian::Query xapQ = toXapianQuery(query.term());
            // The term was not properly converted. Lets abort. The properties
            // must not exist
            if (!query.term().empty() && xapQ.empty()) {
                qDebug() << query.term() << "could not be processed. Aborting";
                return 0;
            }

            xapQ = andQuery(xapQ, convertTypes(query.types()));
            xapQ = andQuery(xapQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
            xapQ = applyIncludeFolder(xapQ, query.includeFolder());
            xapQ = finalizeQuery(xapQ);

            if (xapQ.empty()) {
                // Return all the results
                xapQ = Xapian::Query(std::string());
            }
            Xapian::Enquire enquire(*m_db);
            enquire.set_query(xapQ);

            if (query.sortingOption() == Query::SortNone) {
                enquire.set_weighting_scheme(Xapian::BoolWeight());
            }

            Result& res = m_queryMap[m_nextId++];
            res.mset = enquire.get_mset(query.offset(), query.limit());
            res.it = res.mset.begin();

            return m_nextId-1;
        }
        catch (const Xapian::DatabaseModifiedError&) {
            continue;
        }
        catch (const Xapian::Error&) {
            return 0;
        }
    }
}


