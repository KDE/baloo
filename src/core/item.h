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

#ifndef ITEM_H
#define ITEM_H

#include "core_export.h"

#include <QByteArray>
#include <QMetaType>

namespace Baloo {

class BALOO_CORE_EXPORT Item
{
public:
    Item();
    virtual ~Item();

    typedef QByteArray Id;

    /**
     * Every Item must has a globally unique identifier. Most identifiers
     * are of the form "akonadi:?item=5" or "tag:5" or "file:22456"
     */
    Id id() const;

    /**
     * Sets the id to the desired value. This method should generally never
     * be called by clients. It is used by the data stores.
     */
    void setId(const Id& id);

    /**
     * Every Item has a type that is based on the Item id. It's mostly
     * something as simple as "Email", "Tag", "File", "Contact", etc.
     *
     * This type can be used as a basis of casting this to its appropriate
     * derived class or initializing the derived class with the id.
     */
    virtual QByteArray type() { return QByteArray("Item"); }

    static Item fromId(const Id& id);

private:
    QByteArray m_id;
};

inline Item::Id Item::id() const
{
    return m_id;
}

inline void Item::setId(const Item::Id& id)
{
    m_id = id;
}

//
// Convenience functions
//
QByteArray inline serialize(const QByteArray& namespace_, int id) {
    return namespace_ + ':' + QByteArray::number(id);
}

int inline deserialize(const QByteArray& namespace_, const QByteArray& str) {
    // The +1 is for the ':'
    return str.mid(namespace_.size() + 1).toInt();
}

}

Q_DECLARE_METATYPE(Baloo::Item);

#endif // ITEM_H
