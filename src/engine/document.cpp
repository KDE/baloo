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
{
}

void Document::addTerm(const QByteArray& term, int wdfInc)
{
    m_terms[term].wdf += wdfInc;
}

void Document::addBoolTerm(const QByteArray& term)
{
    m_terms[term].wdf = 0;
}

void Document::addPositionTerm(const QByteArray& term, int position, int wdfInc)
{
    TermData& td = m_terms[term];
    td.wdf += wdfInc;
    td.positions += position;
}

void Document::addXattrPositionTerm(const QByteArray& term, int position, int wdfInc)
{
    TermData& td = m_xattrTerms[term];
    td.wdf += wdfInc;
    td.positions += position;
}

void Document::addXattrTerm(const QByteArray& term, int wdfInc)
{
    m_xattrTerms[term].wdf += wdfInc;
}

void Document::addXattrBoolTerm(const QByteArray& term)
{
    m_xattrTerms[term].wdf = 0;
}

void Document::addFileNamePositionTerm(const QByteArray& term, int position, int wdfInc)
{
    TermData& td = m_fileNameTerms[term];
    td.wdf += wdfInc;
    td.positions += position;
}

void Document::addFileNameTerm(const QByteArray& term, int wdfInc)
{
    m_fileNameTerms[term].wdf += wdfInc;
}

quint64 Document::id() const
{
    return m_id;
}

void Document::setId(quint64 id)
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
