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

#ifndef TAGRELATIONWATCHER_H
#define TAGRELATIONWATCHER_H

#include <QObject>
#include "tag.h"

/**
 * This class can be used to watch any tag or any specific
 * item. Or a specific relation with both tag and item
 */
class VIZIER_TAG_EXPORT TagRelationWatcher : public QObject
{
    Q_OBJECT
public:
    TagRelationWatcher(const Tag& tag, QObject* parent = 0);
    TagRelationWatcher(const Item& item, QObject* parent = 0);
    TagRelationWatcher(const Tag& tag, const Item& item, QObject* parent = 0);

    // FIXME: Maybe we should allow watching multiple tags over here?
    //        There is no extra cost.
signals:
    void tagAdded(const Tag& tag);
    void tagRemoved(const Tag& tag);

    void itemAdded(const Item& item);
    void itemRemoved(const Item& item);

private slots:
    void slotAdded(const QByteArray& tagID, const QByteArray& itemID);
    void slotRemoved(const QByteArray& tagID, const QByteArray& itemID);

    void init();
private:
    QByteArray m_tagID;
    QByteArray m_itemID;
};

#endif // TAGRELATIONWATCHER_H
