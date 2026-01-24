/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "andpostingiterator.h"

#include <ranges>

using namespace Baloo;

AndPostingIterator::AndPostingIterator(std::vector<std::unique_ptr<PostingIterator>> &&iterators)
    : m_iterators(std::move(iterators))
    , m_docId(0)
{
    if (std::ranges::any_of(m_iterators, [](const auto &e) {
            return e == nullptr;
        })) {
        m_iterators.clear();
    }
}

AndPostingIterator::~AndPostingIterator() = default;

quint64 AndPostingIterator::docId() const
{
    return m_docId;
}

quint64 AndPostingIterator::skipTo(quint64 id)
{
    if (m_iterators.empty()) {
        m_docId = 0;
        return 0;
    }

    while (true) {
        quint64 lower_bound = id;
        for (auto &iter : std::as_const(m_iterators)) {
            lower_bound = iter->skipTo(lower_bound);

            if (lower_bound == 0) {
                m_docId = 0;
                return 0;
            }
        }

        if (lower_bound == id) {
            m_docId = lower_bound;
            return lower_bound;
        }
        id = lower_bound;
    }
}

quint64 AndPostingIterator::next()
{
    if (m_iterators.empty()) {
        m_docId = 0;
        return 0;
    }

    m_docId = m_iterators.at(0)->next();
    m_docId = skipTo(m_docId);

    return m_docId;
}
