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

using namespace Baloo;

Document::Document()
    : m_id(0)
    , m_contentIndexing(false)
    , m_mTime(0)
    , m_cTime(0)
{
}

void Document::addTerm(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());
    // This adds "term" without data if it does not exist, otherwise it is a noop
    m_terms[term];
}

void Document::addPositionTerm(const QByteArray& term, int position)
{
    Q_ASSERT(!term.isEmpty());
    TermData& td = m_terms[term];
    td.positions.append(position);
}

void Document::addXattrPositionTerm(const QByteArray& term, int position)
{
    Q_ASSERT(!term.isEmpty());
    TermData& td = m_xattrTerms[term];
    td.positions.append(position);
}

void Document::addXattrTerm(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());
    m_xattrTerms[term];
}

void Document::addFileNamePositionTerm(const QByteArray& term, int position)
{
    Q_ASSERT(!term.isEmpty());
    TermData& td = m_fileNameTerms[term];
    td.positions.append(position);
}

void Document::addFileNameTerm(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());
    m_fileNameTerms[term];
}

quint64 Document::id() const
{
    return m_id;
}

void Document::setId(quint64 id)
{
    Q_ASSERT(id);
    m_id = id;
}

void Document::setUrl(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());
    m_url = url;
}

QByteArray Document::url() const
{
    return m_url;
}

bool Document::contentIndexing() const
{
    return m_contentIndexing;
}

void Document::setContentIndexing(bool val)
{
    m_contentIndexing = val;
}

void Document::setData(const QByteArray& data)
{
    m_data = data;
}
