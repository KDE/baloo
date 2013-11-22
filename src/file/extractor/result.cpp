/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "result.h"

#include <KDebug>
#include <qjson/serializer.h>

#include <QDateTime>

Result::Result()
    : KFileMetaData::ExtractionResult()
{
}

void Result::add(const QString& key_, const QVariant& val)
{
    kDebug() << key_ << val;
    m_map.insert(key_, val);

    QString key = key_.toUpper();

    if (val.type() == QVariant::Bool) {
        m_doc.add_boolean_term(key.toStdString());
    }
    else if (val.type() == QVariant::Int) {
        const QString term = key + val.toString();
        m_doc.add_term(term.toStdString());
    }
    else if (val.type() == QVariant::DateTime) {
        const QString term = key + val.toDateTime().toString(Qt::ISODate);
        m_doc.add_term(term.toStdString());
    }
    else {
        const std::string value = val.toString().toStdString();

        m_termGen.index_text(value);
        m_termGen.index_text(value, 1, key.toStdString());
    }
}

void Result::append(const QString& text)
{
    m_termGen.index_text(text.toStdString());
}

void Result::save(Xapian::WritableDatabase& db)
{
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(m_map);
    m_doc.set_data(json.constData());

    db.replace_document(m_docId, m_doc);
}

void Result::setDocument(const Xapian::Document& doc)
{
    m_doc = doc;
    m_termGen.set_document(m_doc);
}

void Result::setId(uint id)
{
    m_docId = id;
}

uint Result::id() const
{
    return m_docId;
}

