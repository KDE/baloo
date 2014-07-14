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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QQueue>
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include "index.h"

class CollectionIndexingJob;

class JobFactory
{
public:
    virtual ~JobFactory();
    virtual CollectionIndexingJob* createCollectionIndexingJob(Index& index, const Akonadi::Collection& col,
            const QList<Akonadi::Item::Id>& pending, bool fullSync, QObject* parent = 0);
};

/**
 * The scheduler is responsible for scheduling all scheduled tasks.
 *
 * In normal operation this simply involves indexing items and collections that have been added.
 *
 * The scheduler automatically remembers if we failed to index some items before shutting down, and
 * issues a full sync for the affected collections.
 */
class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(Index& index, const QSharedPointer<JobFactory> &jobFactory = QSharedPointer<JobFactory>(), QObject* parent = 0);
    virtual ~Scheduler();
    void addItem(const Akonadi::Item&);
    void scheduleCollection(const Akonadi::Collection&, bool fullSync = false);

    void abort();

    /**
     * Sets the timeout used to detect when a collection is no longer busy (in ms). Used for testing.
     * Default is 5000.
     */
    void setBusyTimeout(int);

Q_SIGNALS:
    void status(int status, const QString& message = QString());
    void percent(int);

public Q_SLOTS:
    void scheduleCompleteSync();

private Q_SLOTS:
    void processNext();
    void slotIndexingFinished(KJob*);
    void slotRootCollectionsFetched(KJob*);

private:
    void collectDirtyCollections();

    QHash<Akonadi::Collection::Id, QQueue<Akonadi::Item::Id> > m_queues;
    QQueue<Akonadi::Collection::Id> m_collectionQueue;
    Index& m_index;
    KJob *m_currentJob;
    QTimer m_processTimer;
    QHash<Akonadi::Collection::Id, qint64> m_lastModifiedTimestamps;
    QSet<Akonadi::Collection::Id> m_dirtyCollections;
    QSharedPointer<JobFactory> m_jobFactory;
    int m_busyTimeout;
};

#endif
