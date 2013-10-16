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

#ifndef ITEM_H
#define ITEM_H

#include "core_export.h"

#include <QMetaType>
#include <QString>

class ItemFetchJob;
class ItemSaveJob;
class ItemCreateJob;
class ItemRemoveJob;

class ItemType;

class VIZIER_CORE_EXPORT Item
{
public:
    Item();
    virtual ~Item();

    /**
     * Every Item must has a globally unique identifier. Most identifiers
     * are of the form "akonadi:?item=5" or "tag:5" or "file:22456"
     */
    QByteArray id() const;

    /**
     * Sets the id to the desired value. This method should generally never
     * be called by clients. It is used by the data stores.
     */
    void setId(const QByteArray& id);

    /**
     * Every Item has a type that is based on the Item id. It's mostly
     * something as simple as "Email", "Tag", "File", "Contact", etc.
     *
     * This type can be used as a basis of casting this to its appropriate
     * derived class or initializing the derived class with the id.
     */
    virtual QByteArray type() { return QByteArray("Item"); }

    virtual ItemFetchJob* fetch();
    virtual ItemSaveJob* save();
    virtual ItemCreateJob* create();
    virtual ItemRemoveJob* remove();

private:
    QByteArray m_id;
};

Q_DECLARE_METATYPE(Item*);

inline QByteArray Item::id() const
{
    return m_id;
}

inline void Item::setId(const QByteArray& id)
{
    m_id = id;
}

#endif // ITEM_H
