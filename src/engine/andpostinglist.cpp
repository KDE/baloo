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

#include "andpostinglist.h"

using namespace Baloo;

AndPostingList::AndPostingList(const PostingList& lhs, const PostingList& rhs)
    : m_lhs(lhs)
    , m_rhs(rhs)
{
}

void AndPostingList::compute()
{
    int i = 0;
    int j = 0;

    while (i < m_lhs.size() && j < m_rhs.size()) {
        const auto& l = m_lhs[i];
        const auto& r = m_rhs[j];

        if (l == r) {
            m_result << l;
            i++;
            j++;
        } else if (l < r) {
            i++;
        } else {
            j++;
        }
    }
}

PostingList AndPostingList::result() const
{
    return m_result;
}


