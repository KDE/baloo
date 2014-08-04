/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
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

#ifndef ABSTRACTINDEXER_H
#define ABSTRACTINDEXER_H

#include <AkonadiCore/Item>
#include <QStringList>

namespace Akonadi {
class Collection;
}

class AbstractIndexer
{
  public:
    AbstractIndexer();
    virtual ~AbstractIndexer();

    virtual QStringList mimeTypes() const = 0;
    virtual void index(const Akonadi::Item& item) = 0;
    virtual void remove(const Akonadi::Item& item) = 0;
    virtual void remove(const Akonadi::Collection& item) = 0;
    virtual void commit() = 0;


    virtual void move(const Akonadi::Item::Id& item,
                      const Akonadi::Entity::Id& from,
                      const Akonadi::Entity::Id& to);
    virtual void updateFlags(const Akonadi::Item& item,
                             const QSet<QByteArray>& addedFlags,
                             const QSet<QByteArray>& removed);
};

#endif // ABSTRACTINDEXER_H
