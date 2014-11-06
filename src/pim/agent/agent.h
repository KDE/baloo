/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#ifndef AGENT_H
#define AGENT_H

#include <akonadi/agentbase.h>
#include <akonadi/collection.h>

#include <QTimer>
#include <QDateTime>
#include <QList>

class KJob;
class AbstractIndexer;

namespace Akonadi
{
class ItemFetchJob;
}

class BalooIndexingAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV3
{
    Q_OBJECT
public:
    BalooIndexingAgent(const QString& id);
    ~BalooIndexingAgent();

    void reindexCollection(const qlonglong id);
    qlonglong indexedItems(const qlonglong id);

    virtual void itemAdded(const Akonadi::Item& item, const Akonadi::Collection& collection);
    virtual void itemChanged(const Akonadi::Item& item, const QSet<QByteArray>& partIdentifiers);
    virtual void itemsFlagsChanged(const Akonadi::Item::List& items,
                                   const QSet<QByteArray>& addedFlags,
                                   const QSet<QByteArray>& removedFlags);
    virtual void itemsRemoved(const Akonadi::Item::List& items);
    virtual void itemsMoved(const Akonadi::Item::List& items,
                            const Akonadi::Collection& sourceCollection,
                            const Akonadi::Collection& destinationCollection);

    virtual void collectionRemoved(const Akonadi::Collection& collection);

    // Remove the entire db
    virtual void cleanup();

private Q_SLOTS:
    void findUnindexedItems();
    void slotRootCollectionsFetched(KJob* job);
    void slotItemFetchFinished(KJob* job);

    void processNext();
    void slotItemsReceived(const Akonadi::Item::List& items);
    void slotCommitTimerElapsed();
    void onAbortRequested();
    void onOnlineChanged(bool online);

private:
    qlonglong indexedItemsInDatabase(const std::string& term, const QString& dbPath) const;
    QDateTime loadLastItemMTime(const QDateTime& defaultDt = QDateTime()) const;
    void createIndexers();
    void addIndexer(AbstractIndexer *indexer);
    AbstractIndexer* indexerForItem(const Akonadi::Item& item) const;
    QList<AbstractIndexer*> indexersForCollection(const Akonadi::Collection& collection) const;

    Akonadi::Item::List m_items;
    QTimer m_timer;
    QDateTime m_lastItemMTime;
    QList<KJob*> m_jobs;

    QList<AbstractIndexer*> m_listIndexer;
    QHash<QString, AbstractIndexer* > m_indexers;

    QTimer m_commitTimer;
    bool m_inProgress;
};

#endif // AGENT_H
