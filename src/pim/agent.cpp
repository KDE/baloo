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

#include "agent.h"

#include "contactindexer.h"
#include "emailindexer.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>

#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>

namespace {
    QString emailIndexingPath() {
        return KStandardDirs::locateLocal("data", "baloo/email/");
    }
    QString contactIndexingPath() {
        return KStandardDirs::locateLocal("data", "baloo/contacts/");
    }
    QString emailContactsIndexingPath() {
        return KStandardDirs::locateLocal("data", "baloo/emailContacts/");
    }
}

BalooIndexingAgent::BalooIndexingAgent(const QString& id)
    : AgentBase(id)
 {
    QTimer::singleShot(0, this, SLOT(findUnindexedItems()));

    createIndexers();

    m_timer.setInterval(10);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(processNext()));

    m_commitTimer.setInterval(1000);
    m_timer.setSingleShot(true);
    connect(&m_commitTimer, SIGNAL(timeout()),
            this, SLOT(slotCommitTimerElapsed()));

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchRemoteIdentification(false);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->setChangeRecordingEnabled(false);
}

BalooIndexingAgent::~BalooIndexingAgent()
{
    while (!m_indexers.isEmpty()) {
        const QMap<QString,AbstractIndexer*>::Iterator it = m_indexers.begin();
        AbstractIndexer *indexer = ( *it );
        Q_FOREACH (const QString &mimeType, indexer->mimeTypes()) {
            m_indexers.remove(mimeType);
        }
        delete indexer;
    }
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
        kError() << "Failed to create email indexer:" << e.get_error_string();
    }

    try {
        indexer = new ContactIndexer(contactIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create contact indexer:" << e.get_error_string();
    }
}

void BalooIndexingAgent::addIndexer(AbstractIndexer* indexer)
{
    Q_FOREACH (const QString& mimeType, indexer->mimeTypes()) {
        m_indexers.insert(mimeType, indexer);
    }
}

AbstractIndexer* BalooIndexingAgent::indexerForItem(const Akonadi::Item& item)
{
    return m_indexers.value(item.mimeType());
}

void BalooIndexingAgent::findUnindexedItems()
{
    KConfig config("baloorc");
    KConfigGroup group = config.group("Akonadi");

    m_lastItemMTime = group.readEntry("lastItem", QDateTime());

    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                        Akonadi::CollectionFetchJob::Recursive);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotRootCollectionsFetched(KJob*)));
    job->start();
        return;
}

void BalooIndexingAgent::slotRootCollectionsFetched(KJob* kjob)
{
    Akonadi::CollectionFetchJob* cjob = qobject_cast<Akonadi::CollectionFetchJob*>(kjob);
    Akonadi::Collection::List cList = cjob->collections();

    m_jobs = 0;
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

        connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
                this, SLOT(slotItemsRecevied(Akonadi::Item::List)));
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotItemFetchFinished(KJob*)));
        job->start();
        m_jobs++;
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
    Q_UNUSED(partIdentifiers);

    AbstractIndexer *indexer = indexerForItem(item);
    if (indexer) {
        indexer->remove(item);
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
        indexer->updateFlags(item, addedFlags, removedFlags);
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
        indexer->remove(item);
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::itemsMoved(const Akonadi::Item::List& items,
                                    const Akonadi::Collection& sourceCollection,
                                    const Akonadi::Collection& destinationCollection)
{
    AbstractIndexer *indexer = indexerForItem(items.first());
    Q_FOREACH (const Akonadi::Item& item, items) {
        indexer->move(item.id(), sourceCollection.id(), destinationCollection.id());
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
    bool initial = group.readEntry("initialIndexingDone", false);
    QDateTime dt = group.readEntry("lastItem", QDateTime::fromMSecsSinceEpoch(1));

    Q_FOREACH (const Akonadi::Item& item, items) {
        AbstractIndexer *indexer = indexerForItem(item);
        if (!indexer) {
            continue;
        }

        indexer->index(item);
        dt = qMax(dt, item.modificationTime());
    }
    if (initial)
        group.writeEntry("lastItem", dt);

    m_commitTimer.start();
}

void BalooIndexingAgent::slotItemFetchFinished(KJob*)
{
    m_jobs--;
    if (m_jobs == 0) {
        KConfig config("baloorc");
        KConfigGroup group = config.group("Akonadi");
        group.writeEntry("initialIndexingDone", true);
    }
}


void BalooIndexingAgent::slotCommitTimerElapsed()
{
    Q_FOREACH (AbstractIndexer *indexer, m_indexers) {
        indexer->commit();
    }
}

AKONADI_AGENT_MAIN(BalooIndexingAgent)
