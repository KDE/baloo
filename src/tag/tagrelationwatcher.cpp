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

#include "tagrelationwatcher.h"

#include <QDBusConnection>
#include <KDebug>

using namespace Baloo;

TagRelationWatcher::TagRelationWatcher(QObject* parent): QObject(parent)
{
    init();
}

TagRelationWatcher::TagRelationWatcher(const Tag& tag, QObject* parent)
    : QObject(parent)
    , m_tagID(tag.id())
{
    init();
}

TagRelationWatcher::TagRelationWatcher(const Item& item, QObject* parent)
    : QObject(parent)
    , m_itemID(item.id())
{
    init();
}

TagRelationWatcher::TagRelationWatcher(const Tag& tag, const Item& item, QObject* parent)
    : QObject(parent)
    , m_tagID(tag.id())
    , m_itemID(item.id())
{
    init();
}

void TagRelationWatcher::init()
{
    QDBusConnection con = QDBusConnection::sessionBus();
    con.connect(QString(), QLatin1String("/tagrelations"), QLatin1String("org.kde"),
                QLatin1String("added"), this, SLOT(slotAdded(QByteArray, QByteArray)));
    con.connect(QString(), QLatin1String("/tagrelations"), QLatin1String("org.kde"),
                QLatin1String("removed"), this, SLOT(slotRemoved(QByteArray, QByteArray)));
}

void TagRelationWatcher::slotAdded(const QByteArray& tagID, const QByteArray& itemID)
{
    if (m_tagID.isEmpty() || m_tagID == tagID) {
        emit tagAdded(Tag::fromId(tagID));
    }

    if (m_itemID.isEmpty() || m_itemID == itemID) {
        Item item;
        item.setId(itemID);

        emit itemAdded(item);
    }
}

void TagRelationWatcher::slotRemoved(const QByteArray& tagID, const QByteArray& itemID)
{
    if (m_tagID.isEmpty() || m_tagID == tagID) {
        emit tagRemoved(Tag::fromId(tagID));
    }

    if (m_itemID.isEmpty() || m_itemID == itemID) {
        Item item;
        item.setId(itemID);

        emit itemRemoved(item);
    }
}
