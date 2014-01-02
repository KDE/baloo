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

#ifndef FILE_H
#define FILE_H

#include "item.h"
#include "filefetchjob.h"
#include <QVariantMap>

namespace Baloo {

class FilePrivate;

class BALOO_FILE_EXPORT File : public Item
{
public:
    File();
    File(const File& f);
    File(const QString& url);
    ~File();

    static File fromId(const Item::Id& id);

    QString url() const;

    /**
     * Gives a variant map of the properties that have been extracted
     * from the file by the indexer
     */
    QVariantMap properties() const;
    QVariant property(const QString& key) const;

    void setRating(int rating);
    int rating() const;

    void addTag(const QString& tag);
    void setTags(const QStringList& tags);
    QStringList tags() const;

    QString userComment() const;
    void setUserComment(const QString& comment);

private:
    FilePrivate* d;
    friend class FileFetchJob;
};

}

#endif // FILE_H
