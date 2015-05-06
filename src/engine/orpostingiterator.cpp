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
{
    for (auto iter : iterators)
        Q_ASSERT(iter);
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
    m_docId = 0;
    if (m_iterators.isEmpty()) {
        return 0;
    }

    for (int i = 0; i < m_iterators.size(); i++) {
        PostingIterator* iter = m_iterators[i];
        if (!iter) {
            continue;
        }

        // First or last element
        if (iter->docId() == 0 && iter->next() == 0) {
            delete m_iterators[i];
            m_iterators[i] = 0;
            continue;
        }

        if (iter->docId() < m_docId || m_docId == 0) {
            m_docId = iter->docId();
        }
    }

    for (int i = 0; i < m_iterators.size(); i++) {
        PostingIterator* iter = m_iterators[i];
        if (iter && iter->docId() <= m_docId) {
            iter->next();
        }
    }

    return m_docId;
}
