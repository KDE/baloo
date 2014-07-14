/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
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

#ifndef INDEX_H
#define INDEX_H
#include <QObject>
#include <QTimer>
#include <AkonadiCore/Collection>
#include <AkonadiCore/item.h>
#include "abstractindexer.h"

/**
 * Maintains the variuous indexers and databases
 */
class Index : public QObject
{
    Q_OBJECT
public:
    explicit Index(QObject* parent = 0);
    virtual ~Index();

    virtual void removeDatabase();
    virtual bool createIndexers();
    virtual void index(const Akonadi::Item& item);
    virtual void move(const Akonadi::Item::List& items,
              const Akonadi::Collection& from,
              const Akonadi::Collection& to);
    virtual void updateFlags(const Akonadi::Item::List& items,
                     const QSet<QByteArray>& addedFlags,
                     const QSet<QByteArray>& removed);
    virtual void remove(const QSet<Akonadi::Item::Id>& ids, const QStringList& mimeTypes);
    virtual void remove(const Akonadi::Item::List& items);
    virtual void remove(const Akonadi::Collection& col);
    virtual bool haveIndexerForMimeTypes(const QStringList &);
    virtual qlonglong indexedItems(const qlonglong id);
    virtual void findIndexed(QSet<Akonadi::Item::Id>& indexed, Akonadi::Collection::Id);
    virtual void scheduleCommit();

public Q_SLOTS:
    virtual void commit();

private:
    void addIndexer(AbstractIndexer* indexer);
    AbstractIndexer* indexerForItem(const Akonadi::Item& item) const;
    QList<AbstractIndexer*> indexersForMimetypes(const QStringList& mimeTypes) const;
    virtual qlonglong indexedItemsInDatabase(const std::string& term, const QString& dbPath) const;
    virtual void findIndexedInDatabase(QSet<Akonadi::Entity::Id>& indexed, Akonadi::Entity::Id collectionId, const QString& dbPath);

    QHash<QString, AbstractIndexer*> m_indexer;
    QTimer m_commitTimer;
};

#endif
