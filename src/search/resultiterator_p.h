/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QUERYITERATOR_H
#define QUERYITERATOR_H

#include "resultiterator.h"
#include "xapian.h"

namespace Baloo {

class ResultIterator::Private
{
public:
    void init(const Xapian::MSet& mset) {
        m_mset = mset;
        m_end = m_mset.end();
        m_iter = m_mset.begin();
        m_firstElement = true;
    }

    Xapian::MSet m_mset;
    Xapian::MSetIterator m_iter;
    Xapian::MSetIterator m_end;

    bool m_firstElement;
};

}

#endif // QUERYITERATOR_H
