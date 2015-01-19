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

#include "document.h"

#include <algorithm>

using namespace Baloo;

Document::Document()
    : m_id(0)
    , m_indexingLevel(0)
{
}

void Document::addTerm(const QByteArray& term)
{
    m_terms << term;
    std::sort(m_terms.begin(), m_terms.end());
}

void Document::addPositionTerm(const QByteArray& term, int position, int wdfInc)
{
    m_terms << term;
    std::sort(m_terms.begin(), m_terms.end());

    // FIXME: We need to start storing this information as well
    Q_UNUSED(position);
    Q_UNUSED(wdfInc);
}

uint Document::id() const
{
    return m_id;
}

void Document::setId(uint id)
{
    m_id = id;
}

void Document::setUrl(const QByteArray& url)
{
    m_url = url;
}

QByteArray Document::url() const
{
    return m_url;
}

int Document::indexingLevel() const
{
    return m_indexingLevel;
}

void Document::setIndexingLevel(int level)
{
    m_indexingLevel = level;
}

bool Document::operator==(const Document& rhs) const
{
    return m_id == rhs.m_id && m_terms == rhs.m_terms && m_url == rhs.m_url
           && m_indexingLevel == rhs.m_indexingLevel;
}
