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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include "tag_export.h"

namespace Baloo {
namespace Tags {

class ConnectionPrivate;

class BALOO_TAG_EXPORT Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject* parent = 0);
    ~Connection();

    /**
     * Returns the default connection for this thread
     */
    static Connection* defaultConnection();

    /**
     * Only used by tests
     */
    explicit Connection(ConnectionPrivate* priv);

    ConnectionPrivate* d;
private:
};

}
}
#endif // CONNECTION_H
