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

#ifndef AGENT_H
#define AGENT_H

#include "emailindexer.h"
#include "contactindexer.h"

#include <akonadi/agentbase.h>
#include <akonadi/collection.h>

#include <QLinkedList>
#include <QTimer>

class KJob;

class BalooIndexingAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV3
{
    Q_OBJECT
public:
    BalooIndexingAgent(const QString& id);
    ~BalooIndexingAgent();

    virtual void itemAdded(const Akonadi::Item& item, const Akonadi::Collection& collection);
    virtual void itemChanged(const Akonadi::Item& item, const QSet<QByteArray>& partIdentifiers);
    virtual void itemsFlagsChanged(const Akonadi::Item::List& items,
                                   const QSet<QByteArray>& addedFlags,
                                   const QSet<QByteArray>& removedFlags);
    virtual void itemsRemoved(const Akonadi::Item::List& items);
    virtual void itemsMoved(const Akonadi::Item::List& items,
                            const Akonadi::Collection& sourceCollection,
                            const Akonadi::Collection& destinationCollection);

    // Remove the entire db
    virtual void cleanup();

private Q_SLOTS:
    void findUnindexedItems();
    void slotRootCollectionsFetched(KJob* job);
    void slotItemFetchFinished(KJob* job);

    void processNext();
    void slotItemsRecevied(const Akonadi::Item::List& items);
    void slotCommitTimerElapsed();

private:
    Akonadi::Item::List m_items;
    QTimer m_timer;
    QDateTime m_lastItemMTime;
    int m_jobs;

    EmailIndexer m_emailIndexer;
    ContactIndexer m_contactIndexer;

    QTimer m_commitTimer;
};

#endif // AGENT_H
