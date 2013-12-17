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

#include "pathfilterpostingsource.h"
#include "filemapping.h"
#include <KDebug>

using namespace Baloo;

PathFilterPostingSource::PathFilterPostingSource(QSqlDatabase* sqlDb, const QString& includeDir)
    : m_sqlDb(sqlDb)
    , m_includeDir(includeDir)
{
    if (!m_includeDir.endsWith('/'))
        m_includeDir.append('/');
}

PathFilterPostingSource::~PathFilterPostingSource()
{
}

void PathFilterPostingSource::init(const Xapian::Database& db)
{
    m_db = db;
    m_iter = db.postlist_begin("");
    m_end = db.postlist_end("");
    m_first = true;
}

bool PathFilterPostingSource::isMatch(uint docid)
{
    FileMapping fileMap(docid);

    if (!fileMap.fetch(*m_sqlDb)) {
        return false;
    }

    if (!fileMap.url().startsWith(m_includeDir)) {
        return false;
    }

    return true;
}

void PathFilterPostingSource::next(Xapian::weight)
{
    do {
        // This has been done so that we do not skip the first element
        // as the PostingSource is supposed to start one before the first element
        // whereas Xapian::Database::postlist_begin gives us the first element
        //
        if (m_first) {
            m_first = false;
        }
        else {
            m_iter++;
        }

        if (m_iter == m_end) {
            return;
        }
    } while (!isMatch(*m_iter));
}

void PathFilterPostingSource::skip_to(Xapian::docid did, Xapian::weight min_wt)
{
    m_iter.skip_to(did);

    if (m_iter == m_end)
        return;

    if (!isMatch(*m_iter))
        next(min_wt);
}


bool PathFilterPostingSource::at_end() const
{
    return m_iter == m_end;
}

Xapian::docid PathFilterPostingSource::get_docid() const
{
    return *m_iter;
}

//
// Term Frequiencies
//
Xapian::doccount PathFilterPostingSource::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount PathFilterPostingSource::get_termfreq_est() const
{
    return m_db.get_doccount();
}

Xapian::doccount PathFilterPostingSource::get_termfreq_max() const
{
    return m_db.get_doccount();
}
