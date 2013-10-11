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

#ifndef RELATION_H
#define RELATION_H

#include "core_export.h"
#include <QByteArray>

class Item;
class ItemType;

class RelationFetchJob;
class RelationCreateJob;
class RelationRemoveJob;

/**
 * This class represents a way of connecting any two Items.
 */
class VIZIER_CORE_EXPORT Relation
{
public:
    virtual ~Relation();

    /**
     * The from must be of type fromType
     */
    virtual QByteArray fromType() const = 0;
    virtual QByteArray toType() const = 0;

    Item from();
    Item to();

    void setFrom(const Item& item);
    void setTo(const Item& to);

    virtual RelationFetchJob* fetch();
    virtual RelationCreateJob* create();
    virtual RelationRemoveJob* remove();

    // Is there any point in allowing modification of a relation?
    // It is supposed to be a lightweight thing
    // virtual RelationModifyJob* save(); // Should we name this commit?
};

#endif // RELATION_H
