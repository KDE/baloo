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

#include "scheduler.h"
#include "collectionindexingjob.h"
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiAgentBase/AgentBase>
#include <AkonadiCore/ServerManager>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QTimer>

JobFactory::~JobFactory()
{
}

CollectionIndexingJob* JobFactory::createCollectionIndexingJob(Index& index, const Akonadi::Collection& col, const QList< Akonadi::Entity::Id >& pending, bool fullSync, QObject* parent)
{
    CollectionIndexingJob *job = new CollectionIndexingJob(index, col, pending, parent);
    job->setFullSync(fullSync);
    return job;
}


Scheduler::Scheduler(Index& index, const QSharedPointer<JobFactory> &jobFactory, QObject* parent)
:   QObject(parent),
    m_index(index),
    m_currentJob(0),
    m_jobFactory(jobFactory),
    m_busyTimeout(5000)
{
    if (!m_jobFactory) {
        m_jobFactory = QSharedPointer<JobFactory>(new JobFactory);
    }
    m_processTimer.setSingleShot(true);
    m_processTimer.setInterval(100);
    connect(&m_processTimer, SIGNAL(timeout()), this, SLOT(processNext()));

    KConfig config(Akonadi::ServerManager::addNamespace(QLatin1String("baloorc")));
    KConfigGroup group = config.group("Akonadi");

    //Schedule collections we know have missing items from last time
    m_dirtyCollections = group.readEntry("dirtyCollections", QList<Akonadi::Collection::Id>()).toSet();
    qDebug() << "Dirty collections " << m_dirtyCollections;
    Q_FOREACH (Akonadi::Collection::Id col, m_dirtyCollections) {
        scheduleCollection(Akonadi::Collection(col), true);
    }

    //Trigger a full sync initially
    if (!group.readEntry("initialIndexingDone", false)) {
        qDebug() << "initial indexing";
        QMetaObject::invokeMethod(this, "scheduleCompleteSync", Qt::QueuedConnection);
    }
    group.writeEntry("initialIndexingDone", true);
    group.sync();
}

Scheduler::~Scheduler()
{
    collectDirtyCollections();
}

void Scheduler::setBusyTimeout(int timeout)
{
    m_busyTimeout = timeout;
}

void Scheduler::collectDirtyCollections()
{
    KConfig config(Akonadi::ServerManager::addNamespace(QLatin1String("baloorc")));
    KConfigGroup group = config.group("Akonadi");
    //Store collections where we did not manage to index all, we'll need to do a full sync for them the next time
    QHash <Akonadi::Entity::Id, QQueue <Akonadi::Entity::Id > >::iterator it = m_queues.begin();
    for (;it != m_queues.end(); it++) {
        if (!it.value().isEmpty()) {
            m_dirtyCollections.insert(it.key());
        }
    }
    qDebug() << m_dirtyCollections;
    group.writeEntry("dirtyCollections", m_dirtyCollections.toList());
    group.sync();
}

void Scheduler::scheduleCollection(const Akonadi::Collection &col, bool fullSync)
{
    if (!m_collectionQueue.contains(col.id())) {
        m_collectionQueue.enqueue(col.id());
    }
    if (fullSync) {
        m_dirtyCollections.insert(col.id());
    }
    processNext();
}

void Scheduler::addItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.parentCollection().isValid());
    m_lastModifiedTimestamps.insert(item.parentCollection().id(), QDateTime::currentMSecsSinceEpoch());
    m_queues[item.parentCollection().id()].append(item.id());
    //Move to the back
    m_collectionQueue.removeOne(item.parentCollection().id());
    m_collectionQueue.enqueue(item.parentCollection().id());
    if (!m_processTimer.isActive()) {
        m_processTimer.start();
    }
}

void Scheduler::scheduleCompleteSync()
{
    qDebug();
    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                        Akonadi::CollectionFetchJob::Recursive);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotRootCollectionsFetched(KJob*)));
    job->start();
}

void Scheduler::slotRootCollectionsFetched(KJob* kjob)
{
    Akonadi::CollectionFetchJob* cjob = static_cast<Akonadi::CollectionFetchJob*>(kjob);
    Q_FOREACH (const Akonadi::Collection& c, cjob->collections()) {
        //For skipping search collections
        if (c.isVirtual()) {
            continue;
        }
        if (c == Akonadi::Collection::root()) {
            continue;
        }
        scheduleCollection(c, true);
    }
}

void Scheduler::abort()
{
    if (m_currentJob) {
        m_currentJob->kill(KJob::Quietly);
    }
    m_currentJob = 0;
    collectDirtyCollections();
    m_collectionQueue.clear();
    status(Akonadi::AgentBase::Idle, i18n("Ready"));
}

void Scheduler::processNext()
{
    m_processTimer.stop();
    if (m_currentJob) {
        return;
    }
    if (m_collectionQueue.isEmpty()) {
        qDebug() << "Processing done";
        status(Akonadi::AgentBase::Idle, i18n("Ready"));
        return;
    }

    //An item was queued within the last 5 seconds, we're probably in the middle of a sync
    const bool collectionIsChanging = (QDateTime::currentMSecsSinceEpoch() - m_lastModifiedTimestamps.value(m_collectionQueue.head())) < m_busyTimeout;
    if (collectionIsChanging) {
        //We're in the middle of something, wait with indexing
        m_processTimer.start();
        return;
    }

    const Akonadi::Collection col(m_collectionQueue.takeFirst());
    qDebug() << "Processing collection: " << col.id();
    QQueue<Akonadi::Item::Id> &itemQueue = m_queues[col.id()];
    const bool fullSync = m_dirtyCollections.contains(col.id());
    CollectionIndexingJob *job = m_jobFactory->createCollectionIndexingJob(m_index, col, itemQueue, fullSync, this);
    itemQueue.clear();
    job->setProperty("collection", col.id());
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotIndexingFinished(KJob*)));
    connect(job, SIGNAL(status(int, QString)), this, SIGNAL(status(int, QString)));
    connect(job, SIGNAL(percent(int)), this, SIGNAL(percent(int)));
    m_currentJob = job;
    job->start();
}

void Scheduler::slotIndexingFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << "Indexing failed: " << job->errorString();
    } else {
        m_dirtyCollections.remove(job->property("collection").value<Akonadi::Collection::Id>());
    }
    m_currentJob = 0;
    m_processTimer.start();
}

