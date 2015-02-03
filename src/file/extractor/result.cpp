/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "result.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDateTime>
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>

// In order to use it in a vector
Result::Result()
    : ExtractionResult(QString(), QString())
    , m_docId(0)
{
}

Result::Result(const QString& url, const QString& mimetype, const Flags& flags)
    : KFileMetaData::ExtractionResult(url, mimetype, flags)
    , m_docId(0)
{
}

void Result::add(KFileMetaData::Property::Property property, const QVariant& value)
{
    QString p = QString::number(static_cast<int>(property));
    if (!value.isNull()) {
        if (!m_map.contains(p)) {
            m_map.insert(p, value);
        } else {
            QVariant prev = m_map.value(p);
            QVariantList list;
            if (prev.type() == QVariant::List) {
                list = prev.toList();
            } else {
                list << prev;
            }

            list << value;
            m_map.insert(p, QVariant(list));
        }
    }

    QString prefix = QLatin1Char('X') + p;

    if (value.type() == QVariant::Bool) {
        //FIXME how do we handle this properly?
        m_doc.addIndexedField(prefix, value.toString());
    }
    else if (value.type() == QVariant::Int) {
        m_doc.addNumericField(prefix, value.toInt());
    }
    else if (value.type() == QVariant::Date) {
        const QString date = value.toDate().toString(Qt::ISODate);
        m_doc.addIndexedField(prefix, date);
    }
    else if (value.type() == QVariant::DateTime) {
        const QString dateTime = value.toDateTime().toString(Qt::ISODate);
        m_doc.addIndexedField(prefix, dateTime);
    }
    else {
        const QString val = value.toString();
        if (val.isEmpty())
            return;

        m_doc.indexText(val, prefix);
        KFileMetaData::PropertyInfo pi(property);
        if (pi.shouldBeIndexed())
            m_doc.indexText(val);
    }
}

void Result::append(const QString& text)
{
    m_doc.indexText(text);
}

void Result::addType(KFileMetaData::Type::Type type)
{
    KFileMetaData::TypeInfo ti(type);
    m_doc.addIndexedField(QStringLiteral("T"), ti.name().toLower());
}

void Result::finish()
{
    QJsonObject jo = QJsonObject::fromVariantMap(m_map);
    QJsonDocument jdoc;
    jdoc.setObject(jo);
    m_doc.addBinaryField(QStringLiteral("prop_json"), jdoc.toJson());
}

void Result::setDocument(const Baloo::LuceneDocument& doc)
{
    m_doc = doc;
}

void Result::setId(uint id)
{
    m_docId = id;
}

uint Result::id() const
{
    return m_docId;
}

QVariantMap Result::map() const
{
    return m_map;
}
