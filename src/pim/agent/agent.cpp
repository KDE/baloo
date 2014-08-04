/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include "agent.h"

#include "contactindexer.h"
#include "emailindexer.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"
#include "balooindexeradaptor.h"

#include "src/file/priority.h"

#include <ItemFetchJob>
#include <ItemFetchScope>
#include <ChangeRecorder>
#include <CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/CollectionStatistics>

#include <AgentManager>
#include <ServerManager>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KGlobal>
#include <KStandardDirs>
#include <KDebug>

#include <QFile>

#define INDEXING_AGENT_VERSION 4

BalooIndexingAgent::BalooIndexingAgent(const QString& id)
    : AgentBase(id),
    m_scheduler(m_index, QSharedPointer<JobFactory>(new JobFactory))
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KConfig config(QLatin1String("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    const int agentIndexingVersion = group.readEntry("agentIndexingVersion", 0);
    if (agentIndexingVersion<INDEXING_AGENT_VERSION) {
        m_index.removeDatabase();
        QTimer::singleShot(0, &m_scheduler, SLOT(scheduleCompleteSync()));
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        group.sync();
    }

    if (!m_index.createIndexers()) {
        Q_EMIT status(Broken, i18nc("@info:status", "No indexers available"));
        setOnline(false);
    } else {
        setOnline(true);
    }
    connect(this, SIGNAL(abortRequested()),
            this, SLOT(onAbortRequested()));
    connect(this, SIGNAL(onlineChanged(bool)),
            this, SLOT(onOnlineChanged(bool)));

    connect(&m_scheduler, SIGNAL(status(int,QString)), this, SIGNAL(status(int,QString)));
    connect(&m_scheduler, SIGNAL(percent(int)), this, SIGNAL(percent(int)));

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchRemoteIdentification(false);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->setChangeRecordingEnabled(false);

    new BalooIndexerAdaptor(this);

    // Cleanup agentsrc after migration to 4.13
    Akonadi::AgentManager* agentManager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List allAgents = agentManager->instances();
    const QStringList oldFeeders = QStringList() << QLatin1String("akonadi_nepomuk_feeder");
    // Cannot use agentManager->instance(oldInstanceName) here, it wouldn't find broken instances.
    Q_FOREACH( const Akonadi::AgentInstance& inst, allAgents ) {
        if ( oldFeeders.contains( inst.identifier() ) ) {
            qDebug() << "Removing old nepomuk feeder" << inst.identifier();
            agentManager->removeInstance( inst );
        }
    }
}

BalooIndexingAgent::~BalooIndexingAgent()
{
}

void BalooIndexingAgent::reindexCollection(const qlonglong id)
{
    
    kDebug() << "Reindexing collection " << id;
    m_scheduler.scheduleCollection(Akonadi::Collection(id), true);
}

qlonglong BalooIndexingAgent::indexedItems(const qlonglong id)
{
    return m_index.indexedItems(id);
}

void BalooIndexingAgent::itemAdded(const Akonadi::Item& item, const Akonadi::Collection& collection)
{
    Q_UNUSED(collection);
    m_scheduler.addItem(item);
}

void BalooIndexingAgent::itemChanged(const Akonadi::Item& item, const QSet<QByteArray>& partIdentifiers)
{
    // We don't index certain parts so we don't care when they change
    QSet<QByteArray> pi = partIdentifiers;
    QMutableSetIterator<QByteArray> it(pi);
    while (it.hasNext()) {
        it.next();
        if (!it.value().startsWith("PLD:"))
            it.remove();
    }

    if (pi.isEmpty()) {
        return;
    }
    m_scheduler.addItem(item);
}

void BalooIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List& items,
                                           const QSet<QByteArray>& addedFlags,
                                           const QSet<QByteArray>& removedFlags)
{
    // Akonadi always sends batch of items of the same type
    m_index.updateFlags(items, addedFlags, removedFlags);
    m_index.scheduleCommit();
}

void BalooIndexingAgent::itemsRemoved(const Akonadi::Item::List& items)
{
    m_index.remove(items);
    m_index.scheduleCommit();
}

void BalooIndexingAgent::collectionRemoved(const Akonadi::Collection& collection)
{
    m_index.remove(collection);
    m_index.scheduleCommit();
}

void BalooIndexingAgent::itemsMoved(const Akonadi::Item::List& items,
                                    const Akonadi::Collection& sourceCollection,
                                    const Akonadi::Collection& destinationCollection)
{
    m_index.move(items, sourceCollection, destinationCollection);
    m_index.scheduleCommit();
}

void BalooIndexingAgent::cleanup()
{
    // Remove all the databases
    Akonadi::AgentBase::cleanup();
}

void BalooIndexingAgent::onAbortRequested()
{
    KConfig config(QLatin1String("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    group.writeEntry("aborted", true);
    group.sync();
    m_scheduler.abort();
}

void BalooIndexingAgent::onOnlineChanged(bool online)
{
    // Ignore everything when offline
    changeRecorder()->setAllMonitored(online);

    // Index items that might have changed while we were offline
    if (online) {
        //We only reindex if this is not a regular start
        KConfig config(QLatin1String("baloorc"));
        KConfigGroup group = config.group("Akonadi");
        const bool aborted = group.readEntry("aborted", false);
        if (aborted) {
            group.writeEntry("aborted", false);
            group.sync();
            m_scheduler.scheduleCompleteSync();
        }
    } else {
        // Abort ongoing indexing when switched to offline
        onAbortRequested();
    }
}

AKONADI_AGENT_MAIN(BalooIndexingAgent)
