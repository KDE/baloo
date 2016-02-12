/*
 * Copyright (c) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef INDEX_H
#define INDEX_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QHash>
#include <QTimer>
#include <QVector>
#include <QQueue>
#include <QDateTime>

#include <Akonadi/Item>

namespace Akonadi {
class Collection;
}

class AbstractIndexer;

class Index : public QObject
{
    Q_OBJECT

public:
    explicit Index(QObject *parent = 0);
    ~Index();

    void stop();

    void schedule(const Akonadi::Item::List &items);
    void schedule(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags,
                  const QSet<QByteArray> &removedFlags);
    void schedule(const Akonadi::Item::List &items, Akonadi::Collection::Id sourceCollection,
                  Akonadi::Collection::Id destCollection);
    void remove(const Akonadi::Item::List &items);
    void remove(const Akonadi::Collection &collection);

    QDateTime lastIndexedItem() const;
    qint64 currentCollection() const;
    QList<qint64> pendingCollections() const;

    static QString emailIndexingPath();
    static QString contactIndexingPath();
    static QString emailContactsIndexingPath();
    static QString akonotesIndexingPath();
    static QString calendarIndexingPath();

private Q_SLOTS:
    void init();
    void destroy();

    void processQueue();

Q_SIGNALS:
    void ready();
    void currentCollectionChanged(qint64 currCollection);

private:
    void commit();
    void startProcessing();

    struct FlagsChange {
        Akonadi::Item item;
        QSet<QByteArray> addedFlags;
        QSet<QByteArray> removedFlags;
    };
    struct MoveChange {
        Akonadi::Item item;
        Akonadi::Collection::Id sourceCollection;
        Akonadi::Collection::Id destCollection;
    };
    struct CollectionQueue {
        QQueue<Akonadi::Item> unindexQueue;
        QQueue<Akonadi::Item> indexQueue;
        QQueue<FlagsChange> flagsQueue;
        QQueue<MoveChange> movesQueue;
    };
    typedef QMap<qint64, CollectionQueue> IndexQueue;

    void processClearCollectionQueue();
    void processCollectionQueues(const CollectionQueue &queue);

    void createIndexers();
    void addIndexer(AbstractIndexer *indexer);
    AbstractIndexer *indexerForItem(const Akonadi::Item &item) const;
    QVector<AbstractIndexer*> indexersForCollection(const Akonadi::Collection &collection) const;

private:
    QHash<QString, AbstractIndexer*> m_indexers;
    QVector<AbstractIndexer*> m_listIndexer;

    IndexQueue m_queue;
    mutable QMutex m_queueLock;
    qint64 m_currentCollection;
    QQueue<Akonadi::Collection> m_removeCollectionQueue;

    QDateTime m_lastIndexedItem;
    QTimer *m_startTimer;
    bool m_running;
    bool m_stop;
};

#endif
