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

#include "andpostingiterator.h"

using namespace Baloo;

AndPostingIterator::AndPostingIterator(const QVector<PostingIterator*>& iterators)
    : m_iterators(iterators)
    , m_docId(0)
{
    if (m_iterators.contains(nullptr)) {
        qDeleteAll(m_iterators);
        m_iterators.clear();
    }
}

AndPostingIterator::~AndPostingIterator()
{
    qDeleteAll(m_iterators);
}

quint64 AndPostingIterator::docId() const
{
    return m_docId;
}

quint64 AndPostingIterator::next()
{
    if (m_iterators.isEmpty()) {
        m_docId = 0;
        return 0;
    }

    if (m_iterators[0]->next() == 0) {
        m_docId = 0;
        return 0;
    }

    m_docId = m_iterators[0]->docId();

    for (int i = 1; i < m_iterators.size(); i++) {
        PostingIterator* iter = m_iterators[i];
        if (iter->docId() == 0 && iter->next() == 0) {
            m_docId = 0;
            return 0;
        }

        iter->skipTo(m_docId);

        if (m_docId != iter->docId()) {
            return next();
        }
    }

    return m_docId;
}
