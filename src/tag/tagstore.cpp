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

#include "tagstore.h"
#include "tag.h"

#include <QDBusConnection>
#include <QCoreApplication>

using namespace Baloo;

TagStore* TagStore::instance()
{
    static TagStore* tagStore = new TagStore(QCoreApplication::instance());
    return tagStore;
}

TagStore::TagStore(QObject* parent): DataStore(parent)
{
    connect(this, SIGNAL(watchStatusChanged(bool)), SLOT(slotWatchStatusChanged(bool)));
}

TagStore::~TagStore()
{
}

void TagStore::slotWatchStatusChanged(bool status)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    if (status) {
        con.connect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("created"), this, SLOT(slotCreated(QByteArray, QString)));
        con.connect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("removed"), this, SLOT(slotRemoved(QByteArray)));
        con.connect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("modified"), this, SLOT(slotModified(QByteArray, QString)));
    }
    else {
        con.disconnect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("created"), this, SLOT(slotCreated(QByteArray, QString)));
        con.disconnect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("removed"), this, SLOT(slotRemoved(QByteArray)));
        con.disconnect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("modified"), this, SLOT(slotModified(QByteArray, QString)));
    }
}

void TagStore::slotCreated(const QByteArray& id, const QString& name)
{
    Tag tag = Tag::fromId(id);
    tag.setName(name);
    // FIXME: Mark the tag as fetched?

    Q_EMIT tagCreated(tag);
    Q_EMIT itemCreated(tag);
}

void TagStore::slotRemoved(const QByteArray& id)
{
    Tag tag = Tag::fromId(id);

    Q_EMIT tagRemoved(tag);
    Q_EMIT itemRemoved(tag);
}

void TagStore::slotModified(const QByteArray& id, const QString& name)
{
    Tag tag = Tag::fromId(id);
    tag.setName(name);

    Q_EMIT tagModified(tag);
}

