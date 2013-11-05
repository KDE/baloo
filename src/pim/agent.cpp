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

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ChangeRecorder>

#include <KStandardDirs>

namespace {
    QString emailIndexingPath() {
        return KStandardDirs::locateLocal("data", "baloo/email/");
    }
}

BalooIndexingAgent::BalooIndexingAgent(const QString& id)
    : AgentBase(id)
    , m_emailIndexer(emailIndexingPath())
{
    QTimer::singleShot(0, this, SLOT(findUnindexedItems()));

    m_timer.setInterval(10);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(processNext()));

    m_commitTimer.setInterval(1000);
    m_timer.setSingleShot(true);
    connect(&m_commitTimer, SIGNAL(timeout()),
            this, SLOT(slotCommitTimerElapsed()));

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->setChangeRecordingEnabled(false);
}

BalooIndexingAgent::~BalooIndexingAgent()
{
}

void BalooIndexingAgent::findUnindexedItems()
{

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

    // We should probably just delete the item and add it again?
    m_emailIndexer.remove(item);
    m_items << item;
    m_timer.start();
}

void BalooIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List& items,
                                           const QSet<QByteArray>& addedFlags,
                                           const QSet<QByteArray>& removedFlags)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        m_emailIndexer.updateFlags(item, addedFlags, removedFlags);
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::itemsRemoved(const Akonadi::Item::List& items)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        m_emailIndexer.remove(item);
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::itemsMoved(const Akonadi::Item::List& items,
                                    const Akonadi::Collection& sourceCollection,
                                    const Akonadi::Collection& destinationCollection)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        m_emailIndexer.move(item.id(), sourceCollection.id(), destinationCollection.id());
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
    job->fetchScope().fetchAllAttributes();
    job->fetchScope().fetchFullPayload();

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotItemsRecevied(Akonadi::Item::List)));
    job->start();
}

void BalooIndexingAgent::slotItemsRecevied(const Akonadi::Item::List& items)
{
    Q_FOREACH (const Akonadi::Item& item, items) {
        if (item.mimeType() == QLatin1String("message/rfc822"))
            m_emailIndexer.index(item);
    }
    m_commitTimer.start();
}

void BalooIndexingAgent::slotCommitTimerElapsed()
{
    m_emailIndexer.commit();
}

AKONADI_AGENT_MAIN(BalooIndexingAgent)
