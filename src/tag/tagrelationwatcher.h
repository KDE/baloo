/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#ifndef TAGRELATIONWATCHER_H
#define TAGRELATIONWATCHER_H

#include <QObject>
#include "tag.h"

namespace Baloo {

/**
 * This class can be used to watch any tag or any specific
 * item. Or a specific relation with both tag and item
 */
class BALOO_TAG_EXPORT TagRelationWatcher : public QObject
{
    Q_OBJECT
public:
    explicit TagRelationWatcher(QObject* parent = 0);
    TagRelationWatcher(const Tag& tag, QObject* parent = 0);
    TagRelationWatcher(const Item& item, QObject* parent = 0);
    TagRelationWatcher(const Tag& tag, const Item& item, QObject* parent = 0);

    // FIXME: Maybe we should allow watching multiple tags over here?
    //        There is no extra cost.
Q_SIGNALS:
    void tagAdded(const Baloo::Tag& tag);
    void tagRemoved(const Baloo::Tag& tag);

    void itemAdded(const Baloo::Item& item);
    void itemRemoved(const Baloo::Item& item);

private Q_SLOTS:
    void slotAdded(const QByteArray& tagID, const QByteArray& itemID);
    void slotRemoved(const QByteArray& tagID, const QByteArray& itemID);

    void init();
private:
    Item::Id m_tagID;
    Item::Id m_itemID;
};

}

#endif // TAGRELATIONWATCHER_H
