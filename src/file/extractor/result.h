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

#ifndef EXTRACTIONRESULT_H
#define EXTRACTIONRESULT_H

#include <kfilemetadata/extractionresult.h>
#include <xapian.h>

class Result : public KFileMetaData::ExtractionResult
{
public:
    Result();

    virtual void add(KFileMetaData::Property::Property property, const QVariant& value);
    virtual void append(const QString& text);
    virtual void addType(KFileMetaData::Type::Type type);

    void save(Xapian::WritableDatabase& db);

    void setId(uint id);
    void setDocument(const Xapian::Document& doc);

    uint id() const;
    QVariantMap map() const;

    Xapian::Document& document() {
        return m_doc;
    }

private:
    uint m_docId;
    Xapian::Document m_doc;
    Xapian::TermGenerator m_termGen;
    Xapian::TermGenerator m_termGenForText;

    QVariantMap m_map;
};

#endif // EXTRACTIONRESULT_H
