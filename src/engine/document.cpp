/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "document.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Baloo;
using namespace Qt::StringLiterals;

Document::Document() = default;

void Document::addTerm(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());
    // This adds "term" without position data if it does not exist, otherwise it is a noop
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

quint64 Document::parentId() const
{
    return m_parentId;
}

void Document::setParentId(quint64 parentId)
{
    Q_ASSERT(parentId);
    m_parentId = parentId;
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

void Document::setMimetype(const QString &mimetype)
{
    m_mimetype = mimetype;
}

QString Document::mimetype() const
{
    return m_mimetype;
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

QByteArray Document::data() const
{
    if (!m_data.isEmpty()) {
        return m_data;
    } else if (m_mimetype.isEmpty()) {
        return {};
    }

    QJsonDocument jdoc;
    QJsonObject mo{{u"M"_s, m_mimetype}};
    jdoc.setObject(mo);
    return jdoc.toJson(QJsonDocument::JsonFormat::Compact);
}
