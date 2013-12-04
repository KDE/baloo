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

private:
    FilePrivate* d;
    friend class FileFetchJob;
};

}

#endif // FILE_H
