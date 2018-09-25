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

#ifndef BALOO_KIO_SEARCH_H_
#define BALOO_KIO_SEARCH_H_

#include <kio/slavebase.h>
#include <KUser>

namespace Baloo
{

class SearchProtocol : public KIO::SlaveBase
{

public:
    SearchProtocol(const QByteArray& poolSocket, const QByteArray& appSocket);
    ~SearchProtocol() override;

    /**
     *
     */
    void listDir(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void mimetype(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void stat(const QUrl& url) override;

private:
    QString getUserName(KUserId uid) const;
    QString getGroupName(KGroupId gid) const;

    mutable QHash<KUserId, QString> mUsercache;
    mutable QHash<KGroupId, QString> mGroupcache;
};
}

#endif
