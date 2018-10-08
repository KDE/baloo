/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#include <QJsonDocument>
#include <QJsonObject>

#include <QDateTime>
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>

// In order to use it in a vector
Result::Result()
    : ExtractionResult(QString(), QString())
    , m_docId(0)
    , m_termGen(nullptr)
    , m_termGenForText(nullptr)
{
}

Result::Result(const QString& url, const QString& mimetype, const Flags& flags)
    : KFileMetaData::ExtractionResult(url, mimetype, flags)
    , m_docId(0)
    , m_termGen(nullptr)
    , m_termGenForText(nullptr)
{
}

void Result::add(KFileMetaData::Property::Property property, const QVariant& value)
{
    int propNum = static_cast<int>(property);
    if (!value.isNull()) {
        QString p = QString::number(propNum);
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

    QByteArray prefix = 'X' + QByteArray::number(propNum) + '-';

    if (value.type() == QVariant::Bool) {
        m_doc.addBoolTerm(prefix);
    }
    else if (value.type() == QVariant::Int) {
        const QByteArray term = prefix + value.toString().toUtf8();
        m_doc.addBoolTerm(term);
    }
    else if (value.type() == QVariant::Date) {
        const QByteArray term = prefix + value.toDate().toString(Qt::ISODate).toUtf8();
        m_doc.addBoolTerm(term);
    }
    else if (value.type() == QVariant::DateTime) {
        const QByteArray term = prefix + value.toDateTime().toString(Qt::ISODate).toUtf8();
        m_doc.addBoolTerm(term);
    }
    else if (value.type() == QVariant::StringList) {
        bool shouldBeIndexed = KFileMetaData::PropertyInfo(property).shouldBeIndexed();
        const auto valueList = value.toStringList();
        for (const auto& val : valueList)
        {
            if (val.isEmpty())
                continue;
            m_termGen.indexText(val, prefix);
            if (shouldBeIndexed)
                m_termGen.indexText(val);

        }
    }
    else {
        const QString val = value.toString();
        if (val.isEmpty())
            return;

        m_termGen.indexText(val, prefix);
        KFileMetaData::PropertyInfo pi(property);
        if (pi.shouldBeIndexed())
            m_termGen.indexText(val);
    }
}

void Result::append(const QString& text)
{
    m_termGenForText.indexText(text);
}

void Result::addType(KFileMetaData::Type::Type type)
{
    KFileMetaData::TypeInfo ti(type);
    const QString t = QLatin1Char('T') + ti.name().toLower();
    m_doc.addBoolTerm(t.toUtf8());
}

void Result::finish()
{
    QJsonObject jo = QJsonObject::fromVariantMap(m_map);
    QJsonDocument jdoc;
    jdoc.setObject(jo);
    m_doc.setData(jdoc.toJson());
}

void Result::setDocument(const Baloo::Document& doc)
{
    m_doc = doc;
    // All document metadata are indexed from position 1000
    m_termGen.setDocument(&m_doc);
    m_termGen.setPosition(1000);

    // All document plain text starts from 10000. This is done to avoid
    // clashes with the term positions
    m_termGenForText.setDocument(&m_doc);
    m_termGenForText.setPosition(10000);
}

quint64 Result::id() const
{
    return m_docId;
}

QVariantMap Result::map() const
{
    return m_map;
}
