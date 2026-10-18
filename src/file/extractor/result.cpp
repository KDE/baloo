/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "result.h"
#include "propertydata.h"
#include "termgenerator.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QDateTime>
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>

using namespace Qt::StringLiterals;

namespace Baloo
{

// In order to use it in a vector
Result::Result()
    : ExtractionResult(QString(), QString())
{
}

Result::Result(const QString& url, const QString& mimetype, const Flags& flags)
    : KFileMetaData::ExtractionResult(url, mimetype, flags)
{
}

void Result::add(KFileMetaData::Property::Property property, const QVariant& value)
{
    if (value.typeId() == QMetaType::QStringList) {
        const auto valueList = value.toStringList();
        for (const auto& val : valueList) {
            m_map.insert(property, val);
        }
    } else {
        m_map.insert(property, value);
    }
}

namespace
{
void indexProperties(Baloo::TermGenerator &termGen, Baloo::Document &doc, const KFileMetaData::PropertyMultiMap &properties)
{
    auto it = properties.cbegin();
    while (it != properties.cend()) {
        const auto property = it.key();
        const auto rangeEnd = properties.upperBound(property);

        const int propNum = static_cast<int>(property);
        const QByteArray prefix = 'X' + QByteArray::number(propNum) + '-';

        while (it != rangeEnd) {
            const auto value = it.value();

            if (value.typeId() == QMetaType::Bool) {
                doc.addTerm(prefix);
            } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::UInt) {
                const QByteArray term = prefix + value.toString().toUtf8();
                doc.addTerm(term);
            } else if (value.typeId() == QMetaType::QDate) {
                const QByteArray term = prefix + value.toDate().toString(Qt::ISODate).toUtf8();
                doc.addTerm(term);
            } else if (value.typeId() == QMetaType::QDateTime) {
                const QByteArray term = prefix + value.toDateTime().toString(Qt::ISODate).toUtf8();
                doc.addTerm(term);
            } else {
                const QString val = value.toString();
                if (!val.isEmpty()) {
                    termGen.indexProperty(val, prefix);
                }
            }
            ++it;
        }
    }
}

void indexText(Baloo::TermGenerator &termGen, const QStringList &text)
{
    for (const auto &block : text) {
        termGen.indexText(block);
    }
}
} // namespace <anonymous>

void Result::append(const QString& text)
{
    m_text.append(text);
}

void Result::addType(KFileMetaData::Type::Type type)
{
    QByteArray num = QByteArray::number(static_cast<int>(type));
    m_doc.addTerm(QByteArray("T") + num);
}

void Result::finish()
{
    QJsonDocument jdoc;
    if (m_map.isEmpty()) {
        QJsonObject mo{{u"M"_s, m_doc.mimetype()}};
        jdoc.setObject(mo);
    } else {
        QJsonObject jo = Baloo::propertyMapToJson(m_map);
        jo.insert(u"M"_s, m_doc.mimetype());
        jdoc.setObject(jo);
    }
    m_doc.setData(jdoc.toJson(QJsonDocument::JsonFormat::Compact));

    Baloo::TermGenerator termGen(m_doc);
    indexProperties(termGen, m_doc, m_map);
    indexText(termGen, m_text);
}

void Result::setDocument(const Baloo::Document& doc)
{
    m_doc = doc;
}

} // namespace Baloo
