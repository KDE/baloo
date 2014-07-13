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
#include "collectionindexingjob.h"

#include "abstractindexer.h"
#include <Akonadi/AgentBase>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ServerManager>
#include <Akonadi/CollectionStatistics>
#include <KLocalizedString>


CollectionIndexingJob::CollectionIndexingJob(Index& index, const Akonadi::Collection& col, const QList< Akonadi::Entity::Id >& pending, QObject* parent)
:   KJob(parent),
    m_collection(col),
    m_pending(pending),
    m_index(index),
    m_reindexingLock(false),
    m_fullSync(true)
{

}

void CollectionIndexingJob::setFullSync(bool enable)
{
    m_fullSync = enable;
}

void CollectionIndexingJob::start()
{
    kDebug();
    m_time.start();

    //Fetch collection for statistics
    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob(m_collection, Akonadi::CollectionFetchJob::Base);
    job->fetchScope().setIncludeStatistics(true);
    job->fetchScope().setIncludeUnsubscribed(true);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotOnCollectionFetched(KJob*)));
    job->start();
}

void CollectionIndexingJob::slotOnCollectionFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    m_collection = static_cast<Akonadi::CollectionFetchJob*>(job)->collections().first();
    Q_EMIT status(Akonadi::AgentBase::Running, i18n("Indexing collection: %1", m_collection.displayName()));
    Q_EMIT percent(0);

    if (!m_index.haveIndexerForMimeTypes(m_collection.contentMimeTypes())) {
        kDebug() << "No indexer for collection, skipping";
        emitResult();
        return;
    }

    if (m_pending.isEmpty()) {
        if (!m_fullSync) {
            kDebug() << "Indexing complete. Total time: " << m_time.elapsed();
            emitResult();
            return;
        }
        findUnindexed();
        return;
    }
    indexItems(m_pending);
}

void CollectionIndexingJob::indexItems(const QList<Akonadi::Item::Id>& itemIds)
{
    Akonadi::Item::List items;
    Q_FOREACH (const Akonadi::Item::Id id, itemIds) {
        items << Akonadi::Item(id);
    }

    Akonadi::ItemFetchJob* fetchJob = new Akonadi::ItemFetchJob(items);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->fetchScope().setCacheOnly(true);
    fetchJob->fetchScope().setIgnoreRetrievalErrors(true);
    fetchJob->fetchScope().setFetchRemoteIdentification(false);
    fetchJob->fetchScope().setFetchModificationTime(true);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    fetchJob->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);
    fetchJob->setProperty("count", items.size());
    fetchJob->setProperty("start", m_time.elapsed());
    m_progressTotal = items.size();
    m_progressCounter = 0;

    connect(fetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotPendingItemsReceived(Akonadi::Item::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(slotPendingIndexed(KJob*)));
    fetchJob->start();
}

void CollectionIndexingJob::slotPendingItemsReceived(const Akonadi::Item::List& items)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        m_index.index(item);
    }
    m_progressCounter++;
    Q_EMIT percent(100.0 * m_progressCounter / m_progressTotal);
}

void CollectionIndexingJob::slotPendingIndexed(KJob *job)
{
    if (job->error()) {
        kWarning() << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    kDebug() << "Indexed " << job->property("count").toInt() << " items in (ms): " << m_time.elapsed() - job->property("start").toInt();

    if (!m_fullSync) {
        m_index.scheduleCommit();
        kDebug() << "Indexing complete. Total time: " << m_time.elapsed();
        emitResult();
        return;
    }

    //We need to commit, otherwise the count is not accurate
    m_index.commit();

    const int start = m_time.elapsed();
    const qlonglong indexedItemsCount = m_index.indexedItems(m_collection.id());
    kDebug() << "Indexed items count took (ms): " << m_time.elapsed() - start;
    kDebug() << "In index: " << indexedItemsCount;
    kDebug() << "In collection: " << m_collection.statistics().count();
    if (m_collection.statistics().count() == indexedItemsCount) {
        kDebug() << "Index up to date";
        emitResult();
        return;
    }

    findUnindexed();
}

void CollectionIndexingJob::findUnindexed()
{
    m_indexedItems.clear();
    m_needsIndexing.clear();
    const int start = m_time.elapsed();
    m_index.findIndexed(m_indexedItems, m_collection.id());
    kDebug() << "Found " << m_indexedItems.size() << " indexed items. Took (ms): " << m_time.elapsed() - start;

    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob(m_collection, this);
    job->fetchScope().fetchFullPayload(false);
    job->fetchScope().setCacheOnly(true);
    job->fetchScope().setIgnoreRetrievalErrors(true);
    job->fetchScope().setFetchRemoteIdentification(false);
    job->fetchScope().setFetchModificationTime(false);
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::None);
    job->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotUnindexedItemsReceived(Akonadi::Item::List)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFoundUnindexed(KJob*)));
    job->start();
}

void CollectionIndexingJob::slotUnindexedItemsReceived(const Akonadi::Item::List& items)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        if (!m_indexedItems.remove(item.id())) {
            m_needsIndexing << item.id();
        }
    }
}

void CollectionIndexingJob::slotFoundUnindexed(KJob *job)
{
    if (job->error()) {
        kWarning() << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    if (!m_indexedItems.isEmpty()) {
        kDebug() << "Removing no longer existing items: " << m_indexedItems.size();
        m_index.remove(m_indexedItems, m_collection.contentMimeTypes());
    }
    if (!m_needsIndexing.isEmpty() && !m_reindexingLock) {
        m_reindexingLock = true; //Avoid an endless loop
        kDebug() << "Found unindexed: " << m_needsIndexing.size();
        indexItems(m_needsIndexing);
        return;
    }

    m_index.commit();
    kDebug() << "Indexing complete. Total time: " << m_time.elapsed();
    emitResult();
}

#include "collectionindexingjob.moc"