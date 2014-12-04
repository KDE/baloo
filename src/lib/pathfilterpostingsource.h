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

#ifndef PATHFILTERPOSTINGSOURCE_H
#define PATHFILTERPOSTINGSOURCE_H

#include <xapian.h>
#include <QString>
#include "xapiandatabase.h"

namespace Baloo {

class PathFilterPostingSource : public Xapian::PostingSource
{
public:
    PathFilterPostingSource(const QString& includeDir);
    virtual ~PathFilterPostingSource();

    virtual void init(const Xapian::Database& db);

    virtual Xapian::docid get_docid() const;

    virtual void next(Xapian::weight min_wt);
    //virtual void skip_to(Xapian::docid did, Xapian::weight min_wt);
    virtual bool at_end() const;

    virtual Xapian::doccount get_termfreq_min() const;
    virtual Xapian::doccount get_termfreq_est() const;
    virtual Xapian::doccount get_termfreq_max() const;

    virtual PostingSource* clone() const {
        return new PathFilterPostingSource(m_includeDir);
    }
private:
    bool isMatch(uint docid);

    QString m_includeDir;

    Xapian::Database m_db;
    Xapian::PostingIterator m_iter;
    Xapian::PostingIterator m_end;

    bool m_first;
};

}

#endif // PATHFILTERPOSTINGSOURCE_H
