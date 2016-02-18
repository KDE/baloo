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

#include "agent.h"
#include "index.h"
#include "balooindexeradaptor.h"

#include "src/file/priority.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/AgentManager>
#include <Akonadi/ServerManager>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <xapian.h>

#include <QFile>

Q_DECLARE_METATYPE(QList<qint64>)

#define INDEXING_AGENT_VERSION 4

BalooIndexingAgent::BalooIndexingAgent(const QString& id)
    : AgentBase(id)
    , m_processedCount(0)
    , m_lastQueueSize(0)
    , m_lastCollection(-1)
{
    qDBusRegisterMetaType<QList<qint64> >();

    m_index = new Index();
    connect(m_index, SIGNAL(ready()), this, SLOT(findUnindexedItems()));
    connect(m_index, SIGNAL(currentCollectionChanged(qint64)),
            this, SLOT(currentIndexingCollectionChanged(qint64)));

    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KConfig config(QLatin1String("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    const int agentIndexingVersion = group.readEntry("agentIndexingVersion", 0);
    if (agentIndexingVersion<INDEXING_AGENT_VERSION) {
        group.deleteEntry("lastItem");
        group.deleteEntry("initialIndexingDone");
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        group.sync();
    }

    connect(this, SIGNAL(abortRequested()),
            this, SLOT(onAbortRequested()));
    connect(this, SIGNAL(onlineChanged(bool)),
            this, SLOT(onOnlineChanged(bool)));

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchRemoteIdentification(false);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->setChangeRecordingEnabled(false);

    new BalooIndexerAdaptor(this);

    // Cleanup agentsrc after migration to 4.13
    Akonadi::AgentManager* agentManager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List allAgents = agentManager->instances();
    const QStringList oldFeeders = QStringList() << QLatin1String("akonadi_nepomuk_feeder");
    // Cannot use agentManager->instance(oldInstanceName) here, it wouldn't find broken instances.
    Q_FOREACH( const Akonadi::AgentInstance& inst, allAgents ) {
        if ( oldFeeders.contains( inst.identifier() ) ) {
            kDebug() << "Removing old nepomuk feeder" << inst.identifier();
            agentManager->removeInstance( inst );
        }
    }
}

BalooIndexingAgent::~BalooIndexingAgent()
{
    // (blocks)
    m_index->stop();

    // Store modification time of the latest item the indexer have seen
    KConfig config(QLatin1String("baloorc"));
    KConfigGroup group = config.group("Akonadi");

    const bool initialDone = group.readEntry("initialIndexingDone", false);
    QDateTime dt = loadLastItemMTime(QDateTime::fromMSecsSinceEpoch(1));
    dt = qMax(dt, m_index->lastIndexedItem());
    if (initialDone) {
        group.writeEntry("lastItem", dt.addSecs(1));
    }

    delete m_index;
}

void BalooIndexingAgent::reindexCollection(const qlonglong id)
{

    if (m_pendingCollections.contains(id)) {
        return;
    }

    m_pendingCollections.insert(id);
    kDebug() << "Reindexing collection " << id;

    Akonadi::CollectionFetchJob *fetch = new Akonadi::CollectionFetchJob(Akonadi::Collection(id),
                                                                         Akonadi::CollectionFetchJob::Base,
                                                                         this);
    fetch->setProperty("collectionId", id);
    connect(fetch, SIGNAL(finished(KJob*)), this, SLOT(slotReindexCollectionFetched(KJob*)));
}

void BalooIndexingAgent::slotReindexCollectionFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        m_pendingCollections.remove(job->property("collectionId").toLongLong());
        return;
    }

    Akonadi::CollectionFetchJob *colFetch = qobject_cast<Akonadi::CollectionFetchJob*>(job);
    if (colFetch->collections().count() != 1) {
        m_pendingCollections.remove(job->property("collectionId").toLongLong());
        return;
    }

    const Akonadi::Collection collection = colFetch->collections().at(0);
    m_index->remove(collection); // unindex the entire collection

    Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob(collection, this);
    fetch->setProperty("collectionsCount", 1);
    fetch->fetchScope().fetchFullPayload(true);
    fetch->fetchScope().setCacheOnly(true);
    fetch->fetchScope().setIgnoreRetrievalErrors(true);
    fetch->fetchScope().setFetchRemoteIdentification(false);
    fetch->fetchScope().setFetchModificationTime(true);
    fetch->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    fetch->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

    connect(fetch, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotItemsReceived(Akonadi::Item::List)));
    connect(fetch, SIGNAL(finished(KJob*)), this, SLOT(slotItemFetchFinished(KJob*)));
    m_jobs << fetch;
}

qlonglong BalooIndexingAgent::indexedItemsInDatabase(const std::string& term, const QString& dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError& e) {
        kError() << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return 0;
    }

    const qlonglong count = db.get_termfreq(term);
    return count;
}

qlonglong BalooIndexingAgent::indexedItems(const qlonglong id)
{
    kDebug() << id;

    const std::string term = QString::fromLatin1("C%1").arg(id).toStdString();
    return indexedItemsInDatabase(term, Index::emailIndexingPath())
            + indexedItemsInDatabase(term, Index::contactIndexingPath())
            + indexedItemsInDatabase(term, Index::akonotesIndexingPath());
}

QList<qint64> BalooIndexingAgent::collectionQueue() const
{
    return m_index->pendingCollections();
}

QDateTime BalooIndexingAgent::loadLastItemMTime(const QDateTime &defaultDt) const
{
    KConfig config(QLatin1String("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    const QDateTime dt = group.readEntry("lastItem", defaultDt);
    //read entry always reads in the local timezone it seems
    return QDateTime(dt.date(), dt.time(), Qt::UTC);
}

void BalooIndexingAgent::findUnindexedItems()
{
    if (!isOnline()) {
        return;
    }

    m_lastItemMTime = loadLastItemMTime();

    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotRootCollectionsFetched(KJob*)));
    job->start();
}

void BalooIndexingAgent::slotRootCollectionsFetched(KJob* kjob)
{
    Akonadi::CollectionFetchJob* cjob = qobject_cast<Akonadi::CollectionFetchJob*>(kjob);
    Akonadi::Collection::List cList = cjob->collections();

    status(Running, i18n("Indexing PIM data"));
    Q_FOREACH (const Akonadi::Collection& c, cList) {
        Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob(c);

        if (!m_lastItemMTime.isNull()) {
            KDateTime dt(m_lastItemMTime, KDateTime::Spec::UTC());
            job->fetchScope().setFetchChangedSince(dt);
        }

        job->fetchScope().fetchFullPayload(true);
        job->fetchScope().setCacheOnly(true);
        job->fetchScope().setIgnoreRetrievalErrors(true);
        job->fetchScope().setFetchRemoteIdentification(false);
        job->fetchScope().setFetchModificationTime(true);
        job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
        job->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

        connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
                this, SLOT(slotItemsReceived(Akonadi::Item::List)));
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotItemFetchFinished(KJob*)));
        job->start();
        m_jobs << job;
    }
}


void BalooIndexingAgent::itemAdded(const Akonadi::Item& item, const Akonadi::Collection& collection)
{
    Q_UNUSED(collection);

    m_index->schedule(Akonadi::Item::List() << item);
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

    Akonadi::Item::List items;
    items << item;
    m_index->remove(items);
    m_index->schedule(items);
}

void BalooIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List& items,
                                           const QSet<QByteArray>& addedFlags,
                                           const QSet<QByteArray>& removedFlags)
{
    m_index->schedule(items, addedFlags, removedFlags);
}

void BalooIndexingAgent::itemsRemoved(const Akonadi::Item::List& items)
{
    m_index->remove(items);
}

void BalooIndexingAgent::itemsMoved(const Akonadi::Item::List& items,
                                    const Akonadi::Collection& sourceCollection,
                                    const Akonadi::Collection& destinationCollection)
{
    m_index->schedule(items, sourceCollection.id(), destinationCollection.id());
}

void BalooIndexingAgent::collectionRemoved(const Akonadi::Collection& collection)
{
    m_index->remove(collection);
}

void BalooIndexingAgent::cleanup()
{
    // Remove all the databases
    Akonadi::AgentBase::cleanup();
}

void BalooIndexingAgent::slotItemsReceived(const Akonadi::Item::List& items)
{
    m_index->schedule(items);
}

void BalooIndexingAgent::slotItemFetchFinished(KJob* job)
{
    m_jobs.removeOne(job);
    if (m_jobs.isEmpty()) {
        KConfig config(QLatin1String("baloorc"));
        KConfigGroup group = config.group("Akonadi");
        group.writeEntry("initialIndexingDone", true);
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        if (m_index->currentCollection() == -1) {
            status(Idle, i18n("Ready"));
        }
    }

    if (job->property("collectionId").isValid()) {
        m_pendingCollections.remove(job->property("collectionId").toLongLong());
    }
}

void BalooIndexingAgent::onAbortRequested()
{
    Q_FOREACH (KJob *job, m_jobs) {
        job->kill(KJob::Quietly);
    }
    m_jobs.clear();
    status(Idle, i18n("Ready"));

    // This might block for a moment, so do it after updating the status
    m_index->stop();
}

void BalooIndexingAgent::onOnlineChanged(bool online)
{
    // Ignore everything when offline
    changeRecorder()->setAllMonitored(online);

    // Index items that might have changed while we were offline
    if (online) {
        findUnindexedItems();
    } else {
        // Abort ongoing indexing when switched to offline
        onAbortRequested();
    }
}

void BalooIndexingAgent::currentIndexingCollectionChanged(qint64 newCol)
{
    Q_EMIT currentCollectionChanged(newCol);

    m_processedCount++;
    const int queueSize = m_index->pendingCollections().count();
    if (queueSize > m_lastQueueSize) {
        percent((float(m_processedCount) / float(queueSize)) * 100.0);
        m_lastQueueSize = queueSize;
    } else if (queueSize == 0) {
        percent(100);
        m_processedCount = 0;
        m_lastQueueSize = 0;
    } else {
        percent((float(m_processedCount - 1) / float(m_lastQueueSize)) * 100);
    }

    if (newCol == -1) {
        status(Idle, i18n("Ready"));
    }
}

AKONADI_AGENT_MAIN(BalooIndexingAgent)
