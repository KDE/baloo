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

#ifndef TAGFETCHJOB_H
#define TAGFETCHJOB_H

#include "tag_export.h"
#include "itemfetchjob.h"

namespace Baloo {

class Tag;

class BALOO_TAG_EXPORT TagFetchJob : public ItemFetchJob
{
    Q_OBJECT
public:
    /**
     * Fetch the data for all the tags in the database
     */
    explicit TagFetchJob(QObject* parent = 0);

    /**
     * Fetch the data for tag \p tag
     */
    TagFetchJob(const Tag& tag, QObject* parent = 0);
    virtual ~TagFetchJob();

    virtual void start();

    enum Error {
        Error_ConnectionError = 1,
        Error_TagInvalidId = 2,
        Error_TagDoesNotExist = 3
    };

Q_SIGNALS:
    void tagReceived(const Baloo::Tag& tag);

private Q_SLOTS:
    void doStart();

private:
    class Private;
    Private* d;
};

}

#endif // TAGFETCHJOB_H
