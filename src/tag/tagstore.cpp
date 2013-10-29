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

TagFetchJob* TagStore::fetchAll()
{
    return new TagFetchJob(this);
}

void TagStore::slotWatchStatusChanged(bool status)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    if (status) {
        con.connect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("created"), this, SLOT(slotCreated(QByteArray, QString)));
        con.connect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("removed"), this, SLOT(slotRemoved(QByteArray)));
    }
    else {
        con.disconnect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("created"), this, SLOT(slotCreated(QByteArray, QString)));
        con.disconnect(QString(), QLatin1String("/tags"), QLatin1String("org.kde"),
                    QLatin1String("removed"), this, SLOT(slotRemoved(QByteArray)));
    }
}

void TagStore::slotCreated(const QByteArray& id, const QString& name)
{
    Tag tag = Tag::fromId(id);
    tag.setName(name);
    // FIXME: Mark the tag as fetched?

    emit tagCreated(tag);
    emit itemCreated(tag);
}

void TagStore::slotRemoved(const QByteArray& id)
{
    Tag tag = Tag::fromId(id);

    emit tagRemoved(tag);
    emit itemRemoved(tag);
}
