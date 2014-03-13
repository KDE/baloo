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

#include "contactindexer.h"
#include "emailindexer.h"
#include "akonotesindexer.h"
#include "balooindexeradaptor.h"

#include "src/file/priority.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/AgentManager>
#include <Akonadi/ServerManager>

#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

namespace {
    QString dbPath(const QString &dbName) {
        QString basePath = "baloo";
        if (Akonadi::ServerManager::hasInstanceIdentifier()) {
            basePath = QString::fromLatin1("baloo/instances/%1").arg(Akonadi::ServerManager::instanceIdentifier());
        }
        return KStandardDirs::locateLocal("data", QString::fromLatin1("%1/%2/").arg(basePath, dbName));
    }
    QString emailIndexingPath() {
        return dbPath("email");
    }
    QString contactIndexingPath() {
        return dbPath("contacts");
    }
    QString emailContactsIndexingPath() {
        return dbPath("emailContacts");
    }
    QString akonotesIndexingPath() {
        return dbPath("notes");
    }
}

#define INDEXING_AGENT_VERSION 2

BalooIndexingAgent::BalooIndexingAgent(const QString& id)
    : AgentBase(id),
      m_inProgress(false)
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KConfig config("baloorc");
    KConfigGroup group = config.group("Akonadi");
    const int agentIndexingVersion = group.readEntry("agentIndexingVersion", 0);
    if (agentIndexingVersion<INDEXING_AGENT_VERSION) {
        group.deleteEntry("lastItem");
        group.deleteEntry("initialIndexingDone");
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        group.sync();
    }

    QTimer::singleShot(0, this, SLOT(findUnindexedItems()));

    createIndexers();
    if (m_indexers.isEmpty()) {
        Q_EMIT status(Broken, i18nc("@info:status", "No indexers available"));
        setOnline(false);
    } else {
        setOnline(true);
    }
    connect(this, SIGNAL(abortRequested()),
            this, SLOT(onAbortRequested()));
    connect(this, SIGNAL(onlineChanged(bool)),
            this, SLOT(onOnlineChanged(bool)));

    m_timer.setInterval(10);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(processNext()));

    m_commitTimer.setInterval(1000);
    m_commitTimer.setSingleShot(true);
    connect(&m_commitTimer, SIGNAL(timeout()),
            this, SLOT(slotCommitTimerElapsed()));

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
    const QStringList oldFeeders = QStringList() << "akonadi_nepomuk_feeder";
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
    qDeleteAll(m_indexers.values().toSet());
    m_indexers.clear();
}

void BalooIndexingAgent::reindexCollection(const qlonglong id)
{
    
    kDebug() << "Reindexing collection " << id;
}

qlonglong BalooIndexingAgent::indexedItemsInDatabase(const std::string& term, const QString& dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(dbPath.toStdString());
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
    return indexedItemsInDatabase(term, emailIndexingPath())
            + indexedItemsInDatabase(term, contactIndexingPath())
            + indexedItemsInDatabase(term, akonotesIndexingPath());
}

void BalooIndexingAgent::createIndexers()
{
    AbstractIndexer *indexer = 0;
    try {
        indexer = new EmailIndexer(emailIndexingPath(), emailContactsIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create email indexer:" << QString::fromStdString(e.get_msg());
    }

    try {
        indexer = new ContactIndexer(contactIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create contact indexer:" << QString::fromStdString(e.get_msg());
    }

    try {
        indexer = new AkonotesIndexer(akonotesIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    }
}

void BalooIndexingAgent::addIndexer(AbstractIndexer* indexer)
{
    Q_FOREACH (const QString& mimeType, indexer->mimeTypes()) {
        m_indexers.insert(mimeType, indexer);
    }
}

AbstractIndexer* BalooIndexingAgent::indexerForItem(const Akonadi::Item& item) const
{
    return m_indexers.value(item.mimeType());
}

QList<AbstractIndexer*> BalooIndexingAgent::indexersForCollection(const Akonadi::Collection& collection) const
{
    QList<AbstractIndexer*> indexers;
    Q_FOREACH (const QString& mimeType, collection.contentMimeTypes()) {
        AbstractIndexer *i = m_indexers.value(mimeType);
        if (i) {
            indexers.append(i);
        }
    }
    return indexers;
}

QDateTime BalooIndexingAgent::loadLastItemMTime(const QDateTime &defaultDt) const
{
    KConfig config("baloorc");
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
    if (m_inProgress) {
        return;
    }
    m_inProgress = true;

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
        job->setProperty("collectionsCount", cList.size());

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
        job->setDeliveryOption( Akonadi::ItemFetchJob::EmitItemsInBatches );

        connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
                this, SLOT(slotItemsRecevied(Akonadi::Item::List)));
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotItemFetchFinished(KJob*)));
        job->start();
        m_jobs << job;
    }
}


void BalooIndexingAgent::itemAdded(const Akonadi::Item& item, const Akonadi::Collection& collection)
{
    Q_UNUSED(collection);

    m_items << item;
    m_timer.start();
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

    if (pi.isEmpty())
        return;

    AbstractIndexer *indexer = indexerForItem(item);
    if (indexer) {
        try {
            indexer->remove(item);
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
        m_items << item;
        m_timer.start();
    }
}

void BalooIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List& items,
                                           const QSet<QByteArray>& addedFlags,
                                           const QSet<QByteArray>& removedFlags)
{
    // Akonadi always sends batch of items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }

    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->updateFlags(item, addedFlags, removedFlags);
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::itemsRemoved(const Akonadi::Item::List& items)
{
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }

    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->remove(item);
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::collectionRemoved(const Akonadi::Collection& collection)
{
    Q_FOREACH (AbstractIndexer *indexer, indexersForCollection(collection)) {
        try {
            indexer->remove(collection);
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::itemsMoved(const Akonadi::Item::List& items,
                                    const Akonadi::Collection& sourceCollection,
                                    const Akonadi::Collection& destinationCollection)
{
    AbstractIndexer *indexer = indexerForItem(items.first());
    Q_FOREACH (const Akonadi::Item& item, items) {
        try {
            indexer->move(item.id(), sourceCollection.id(), destinationCollection.id());
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::cleanup()
{
    // Remove all the databases
    Akonadi::AgentBase::cleanup();
}

void BalooIndexingAgent::processNext()
{
    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob(m_items);
    m_items.clear();
    job->fetchScope().fetchFullPayload(true);
    job->fetchScope().setCacheOnly(true);
    job->fetchScope().setIgnoreRetrievalErrors(true);
    job->fetchScope().setFetchRemoteIdentification(false);
    job->fetchScope().setFetchModificationTime(true);
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotItemsRecevied(Akonadi::Item::List)));
    job->start();
}

void BalooIndexingAgent::slotItemsRecevied(const Akonadi::Item::List& items)
{
    KConfig config("baloorc");
    KConfigGroup group = config.group("Akonadi");

    const bool initialDone = group.readEntry("initialIndexingDone", false);
    QDateTime dt = loadLastItemMTime(QDateTime::fromMSecsSinceEpoch(1));

    Q_FOREACH (const Akonadi::Item& item, items) {
        AbstractIndexer *indexer = indexerForItem(item);
        if (!indexer) {
            continue;
        }

        try {
            indexer->index(item);
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }

        dt = qMax(dt, item.modificationTime());
    }
    if (initialDone)
        group.writeEntry("lastItem", dt);

    m_commitTimer.start();
}

void BalooIndexingAgent::slotItemFetchFinished(KJob* job)
{
    const int totalJobs = job->property("collectionsCount").toInt();
    m_jobs.removeOne(job);
    percent((float(totalJobs - m_jobs.count()) / float(totalJobs)) * 100);
    if (m_jobs.isEmpty()) {
        KConfig config("baloorc");
        KConfigGroup group = config.group("Akonadi");
        group.writeEntry("initialIndexingDone", true);
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        status(Idle, i18n("Ready"));
        m_inProgress = false;
    }
}


void BalooIndexingAgent::slotCommitTimerElapsed()
{
    Q_FOREACH (AbstractIndexer *indexer, m_indexers) {
        try {
            indexer->commit();
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void BalooIndexingAgent::onAbortRequested()
{
    Q_FOREACH (KJob *job, m_jobs) {
        job->kill(KJob::Quietly);
    }
    m_jobs.clear();
    m_inProgress = false;
    status(Idle, i18n("Ready"));
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

AKONADI_AGENT_MAIN(BalooIndexingAgent)
