/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "orpostingiterator.h"

using namespace Baloo;

OrPostingIterator::OrPostingIterator(const QVector<PostingIterator*>& iterators)
    : m_iterators(iterators)
    , m_docId(0)
    , m_nextId(0)
{
    for (auto it = m_iterators.begin(), end = m_iterators.end(); it != end;) {
        /*
         * Check for null iterators
         * Preferably, these are not pushed to the list at all, but better be safe
         */
        if (!(*it)) {
            it = m_iterators.erase(it);
            continue;
        }

        auto docId = (*it)->next();
        // find smallest docId
        if (docId && (docId < m_nextId || m_nextId == 0)) {
            m_nextId = docId;
        }

        it++;
    }
}

OrPostingIterator::~OrPostingIterator()
{
    qDeleteAll(m_iterators);
}

quint64 OrPostingIterator::docId() const
{
    return m_docId;
}

quint64 OrPostingIterator::next()
{
    m_docId = m_nextId;
    m_nextId = 0;

    if (!m_docId) {
        return 0;
    }

    // advance all iterators which point to the lowest docId
    for (auto it = m_iterators.begin(); it != m_iterators.end(); ) {
        PostingIterator* iter = *it;

        auto docId = iter->docId();
        if (docId <= m_docId) {
            docId = iter->next();
            // remove element if iterator has reached the end
            if (docId == 0) {
                delete iter;
                *it = nullptr;
                it = m_iterators.erase(it);
                continue;
            }
        }

        // check if the docId is the new lowest docId
        if ((docId < m_nextId) || (m_nextId == 0)) {
            m_nextId = docId;
        }

        it++;
    }

    return m_docId;
}
