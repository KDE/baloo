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

#ifndef DATASTORE_H
#define DATASTORE_H

#include <QObject>
#include "core_export.h"

namespace Baloo {

class Item;
class ItemFetchJob;

// FIXME: Maybe this should be a singleton?
class BALOO_CORE_EXPORT DataStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool watchEnabled READ watchEnabled WRITE setWatchEnabled)

public:
    explicit DataStore(QObject* parent = 0);
    virtual ~DataStore();

    // Maybe this doesn't always need to implemented?
    // Implement some concept of features?
    /**
     * Fetches all the items implemented in this store. Depending
     * on the store this can get quite expensive
     */
    virtual ItemFetchJob* fetchAll();

    /**
     * Enable watching for new items in this store.
     * Not all stores support watching.
     */
    void setWatchEnabled(bool status);
    bool watchEnabled() const;

    /**
     * Specifies if this specific store supports watching for
     * the creation and removal of items
     */
    virtual bool supportsWatching() const;

Q_SIGNALS:
    void itemCreated(const Item& item);
    void itemRemoved(const Item& item);

    void watchStatusChanged(bool watchEnabled);

private:
    bool m_watchEnabled;
};

}

#endif // DATASTORE_H
