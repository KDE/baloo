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

#include "searchstore.h"

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <QThreadStorage>
#include <QMutex>

using namespace Baloo;

SearchStore::SearchStore(QObject* parent)
    : QObject(parent)
{
}

SearchStore::~SearchStore()
{
}

QUrl SearchStore::url(int)
{
    return QUrl();
}

QString SearchStore::icon(int)
{
    return QString();
}

QString SearchStore::text(int)
{
    return QString();
}

QString SearchStore::property(int, const QString&)
{
    return QString();
}

//
// Search Stores
//
// static
QList<SearchStore*> SearchStore::searchStores()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    // Get all the plugins
    KService::List plugins = KServiceTypeTrader::self()->query("BalooSearchStore");

    QList<Baloo::SearchStore*> stores;
    KService::List::const_iterator it;
    for (it = plugins.constBegin(); it != plugins.constEnd(); it++) {
        KService::Ptr service = *it;

        QString error;
        Baloo::SearchStore* st = service->createInstance<Baloo::SearchStore>(0, QVariantList(), &error);
        if (!st) {
            kError() << "Could not create SearchStore: " << service->library();
            kError() << error;
            continue;
        }

        stores << st;
    }

    return stores;
}
