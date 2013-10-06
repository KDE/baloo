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

#include <QString>

class ItemFetchJob;
class ItemSaveJob;
class ItemCreateJob;
class ItemRemoveJob;

class ItemType;

class Item
{
public:
    virtual ~Item();

    /**
     * Every item has a unique identifier
     */
    virtual QString id();

    virtual QString type();

    /**
     * Fetches a property value from the local cache. This cache
     * is empty until load has been called
     */
    //FIXME: The key should not be a string
    virtual QVariant property(QString key);

    /**
     * This sets the property in the local cache. It is not actually
     * saved until you call save
     */
    //FIXME: The key should not be a string
    virtual void setProperty(QString key, const QVariant& value):

    virtual ItemFetchJob* fetch();
    virtual ItemSaveJob* save();
    virtual ItemCreateJob* create();
    virtual ItemRemoveJob* remove();
};

#endif // ITEM_H
