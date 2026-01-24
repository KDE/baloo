/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "orpostingiterator.h"

using namespace Baloo;

OrPostingIterator::OrPostingIterator(std::vector<std::unique_ptr<PostingIterator>> &&iterators)
    : m_iterators(std::move(iterators))
    , m_docId(0)
    , m_nextId(0)
{
    /*
     * Check for null iterators
     * Preferably, these are not pushed to the list at all, but better be safe
     */
    std::erase_if(m_iterators, [](const auto &e) {
        return e == nullptr;
    });

    for (auto &iter : std::as_const(m_iterators)) {
        auto docId = iter->next();
        // find smallest docId
        if (docId && (docId < m_nextId || m_nextId == 0)) {
            m_nextId = docId;
        }
    }
}

OrPostingIterator::~OrPostingIterator() = default;

quint64 OrPostingIterator::docId() const
{
    return m_docId;
}

quint64 OrPostingIterator::skipTo(quint64 id)
{
    if (m_docId >= id) {
        return m_docId;
    }
    if (m_nextId == 0) {
        m_docId = m_nextId;
        return 0;
    }

    if (id > m_nextId) {
        // Fast forward - move all iterators to the lowest position
        // greater or equal to id
        m_nextId = 0;
        for (auto &iter : std::as_const(m_iterators)) {
            auto docId = iter->skipTo(id);
            if (docId > 0) {
                if (docId < m_nextId || !m_nextId) {
                    m_nextId = docId;
                }
            }
        }
        if (m_nextId == 0) {
            m_docId = m_nextId;
            return 0;
        }
    }

    m_docId = m_nextId;
    m_nextId = 0;

    // advance all iterators which point to the lowest docId
    for (auto &iter : m_iterators) {
        auto docId = iter->docId();
        if (docId == m_docId) {
            docId = iter->next();
        }

        if (docId == 0) {
            // remove element if iterator has reached the end
            iter.reset();
        } else {
            // check if the docId is the new lowest docId
            if (docId < m_nextId || !m_nextId) {
                m_nextId = docId;
            }
        }
    }
    std::erase_if(m_iterators, [](const auto &e) {
        return e == nullptr;
    });

    return m_docId;
}

quint64 OrPostingIterator::next()
{
    if (m_nextId) {
        m_docId = skipTo(m_nextId);
    } else {
        m_docId = 0;
    }
    return m_docId;
}
