/*
 * Copyright (c) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "index.h"
#include "emailindexer.h"
#include "contactindexer.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"

#include <QThread>
#include <QMutexLocker>
#include <QDir>

#include <Akonadi/Item>
#include <Akonadi/Collection>
#include <Akonadi/ServerManager>

#include <KStandardDirs>

#include <unistd.h>

namespace {
    QString dbPath(const QString& dbName) {
        QString basePath = QLatin1String("baloo");
        if (Akonadi::ServerManager::hasInstanceIdentifier()) {
            basePath = QString::fromLatin1("baloo/instances/%1").arg(Akonadi::ServerManager::instanceIdentifier());
        }
        return KGlobal::dirs()->localxdgdatadir() + QString::fromLatin1("%1/%2/").arg(basePath, dbName);
    }
}

Index::Index(QObject *parent)
    : QObject(parent)
    , m_currentCollection(-1)
    , m_startTimer(0)
    , m_running(false)
    , m_stop(false)
{
    QThread *thread = new QThread;
    moveToThread(thread);
    thread->start();

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

Index::~Index()
{
    const bool invoke = QMetaObject::invokeMethod(this, "destroy", Qt::QueuedConnection);
    Q_ASSERT(invoke); Q_UNUSED(invoke);
    if (!thread()->wait(10 * 1000)) {
        thread()->terminate();
        thread()->wait();
    }
    delete thread();
}

void Index::stop()
{
    QMutexLocker locker(&m_queueLock);
    QMetaObject::invokeMethod(m_startTimer, "stop", Qt::BlockingQueuedConnection);
    if (!m_running) {
        return;
    }
    m_stop = true;
    locker.unlock();

    // Wait for indexer to stop
    Q_FOREVER {
        locker.relock();
        if (!m_running) {
            return;
        }
        locker.unlock();
        usleep(10000);
    }
}

void Index::init()
{
    m_startTimer = new QTimer(this);
    m_startTimer->setInterval(500);
    m_startTimer->setSingleShot(true);
    connect(m_startTimer, SIGNAL(timeout()), this, SLOT(processQueue()));

    createIndexers();

    Q_EMIT ready();
}

void Index::destroy()
{
    delete m_startTimer;
    // Will also commit pending data
    qDeleteAll(m_listIndexer);

    thread()->quit();
}

QString Index::emailIndexingPath() {
    return dbPath(QLatin1String("email"));
}
QString Index::contactIndexingPath() {
    return dbPath(QLatin1String("contacts"));
}
QString Index::emailContactsIndexingPath() {
    return dbPath(QLatin1String("emailContacts"));
}
QString Index::akonotesIndexingPath() {
    return dbPath(QLatin1String("notes"));
}
QString Index::calendarIndexingPath() {
    return dbPath(QLatin1String("calendars"));
}

QDateTime Index::lastIndexedItem() const
{
    QMutexLocker locker(&m_queueLock);
    return m_lastIndexedItem;
}

qint64 Index::currentCollection() const
{
    QMutexLocker locker(&m_queueLock);
    return m_currentCollection;
}

QList<qint64> Index::pendingCollections() const
{
    QMutexLocker locker(&m_queueLock);
    return m_queue.uniqueKeys();
}

void Index::createIndexers()
{
    AbstractIndexer *indexer = 0;
    try {
        QDir().mkpath(emailIndexingPath());
        QDir().mkpath(emailContactsIndexingPath());
        indexer = new EmailIndexer(emailIndexingPath(), emailContactsIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create email indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        kError() << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(contactIndexingPath());
        indexer = new ContactIndexer(contactIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create contact indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        kError() << "Random exception, but we do not want to crash";
    }


    try {
        QDir().mkpath(akonotesIndexingPath());
        indexer = new AkonotesIndexer(akonotesIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        kError() << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(calendarIndexingPath());
        indexer = new CalendarIndexer(calendarIndexingPath());
        addIndexer(indexer);
    }
    catch (const Xapian::DatabaseError &e) {
        delete indexer;
        kError() << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        kError() << "Random exception, but we do not want to crash";
    }
}

void Index::addIndexer(AbstractIndexer *indexer)
{
    m_listIndexer.append(indexer);
    Q_FOREACH (const QString& mimeType, indexer->mimeTypes()) {
        m_indexers.insert(mimeType, indexer);
    }
}

AbstractIndexer *Index::indexerForItem(const Akonadi::Item &item) const
{
    return m_indexers.value(item.mimeType());
}

QVector<AbstractIndexer*> Index::indexersForCollection(const Akonadi::Collection &collection) const
{
    QVector<AbstractIndexer*> indexers;
    Q_FOREACH (const QString &mimeType, collection.contentMimeTypes()) {
        AbstractIndexer *i = m_indexers.value(mimeType);
        if (i) {
            indexers.append(i);
        }
    }
    return indexers;
}

void Index::schedule(const Akonadi::Item::List &items)
{
    qint64 parentId = -1;
    IndexQueue::Iterator iter;
    QMutexLocker locker(&m_queueLock);
    Q_FOREACH (const Akonadi::Item &item, items) {
        if (item.parentCollection().id() != parentId) {
            parentId = item.parentCollection().id();
            iter = m_queue.find(parentId);
        }

        if (iter == m_queue.end()) {
            CollectionQueue cq;
            cq.indexQueue.enqueue(item);
            iter = m_queue.insert(parentId, cq);
        } else {
            iter->indexQueue.enqueue(item);
        }
    }

    startProcessing();
}

void Index::schedule(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags,
                     const QSet<QByteArray> &removedFlags)
{
    FlagsChange fc;
    fc.addedFlags = addedFlags;
    fc.removedFlags = removedFlags;

    qint64 parentId = -1;
    IndexQueue::Iterator iter;
    QMutexLocker locker(&m_queueLock);
    Q_FOREACH (const Akonadi::Item &item, items) {
        if (item.parentCollection().id() != parentId) {
            parentId = item.parentCollection().id();
            iter = m_queue.find(parentId);
        }

        fc.item = item;
        if (iter == m_queue.end()) {
            CollectionQueue cq;
            cq.flagsQueue.enqueue(fc);
            iter = m_queue.insert(parentId, cq);
        } else {
            iter->flagsQueue.enqueue(fc);
        }
    }

    startProcessing();
}

void Index::schedule(const Akonadi::Item::List &items,
                     Akonadi::Collection::Id sourceCollection,
                     Akonadi::Collection::Id destCollection)
{
    MoveChange mc;
    mc.sourceCollection = sourceCollection;
    mc.destCollection = destCollection;

    qint64 parentId = -1;
    IndexQueue::Iterator iter;
    QMutexLocker locker(&m_queueLock);
    Q_FOREACH (const Akonadi::Item &item, items) {
        if (destCollection != parentId) {
            parentId = destCollection;
            iter = m_queue.find(destCollection);
        }

        mc.item = item;
        if (iter == m_queue.end()) {
            CollectionQueue cq;
            cq.movesQueue.enqueue(mc);
            iter = m_queue.insert(destCollection, cq);
        } else {
            iter->movesQueue.enqueue(mc);
        }
    }

    startProcessing();
}

void Index::remove(const Akonadi::Item::List &items)
{
    qint64 parentId = -1;
    IndexQueue::Iterator iter;
    QMutexLocker locker(&m_queueLock);
    Q_FOREACH (const Akonadi::Item &item, items) {
        if (item.parentCollection().id() != parentId) {
            parentId = item.parentCollection().id();
            iter = m_queue.find(parentId);
        }

        if (iter == m_queue.end()) {
            CollectionQueue cq;
            cq.unindexQueue.enqueue(item);
            iter = m_queue.insert(parentId, cq);
        } else {
            iter->unindexQueue.enqueue(item);
        }
    }

    startProcessing();
}

void Index::remove(const Akonadi::Collection &collection)
{
    QMutexLocker locker(&m_queueLock);
    if (!m_removeCollectionQueue.contains(collection)) {
        m_removeCollectionQueue.append(collection);
    }

    startProcessing();
}

void Index::startProcessing()
{
    // WARNING: This assumes m_queueLock is locked!

    if (m_running) {
        return;
    }

    if (!m_startTimer->isActive()) {
        // start timer in the right thread
        QMetaObject::invokeMethod(m_startTimer, "start", Qt::QueuedConnection);
    }
}

void Index::commit()
{
    Q_FOREACH (AbstractIndexer *indexer, m_listIndexer) {
        try {
            indexer->commit();
        } catch (const Xapian::Error &e) {
            kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::processClearCollectionQueue()
{
    QMutexLocker locker(&m_queueLock);
    while (!m_removeCollectionQueue.isEmpty()) {
        const Akonadi::Collection &collection = m_removeCollectionQueue.dequeue();
        locker.unlock();

        kDebug() << "Clearing collection" << collection.id();
        Q_FOREACH (AbstractIndexer *indexer, indexersForCollection(collection)) {
            if (indexer) {
                try {
                    indexer->remove(collection);
                } catch (const Xapian::Error &e) {
                    kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
                }
            }
        }
        kDebug() << "Done";

        locker.relock();
        if (m_stop) {
            return;
        }
    }
}

void Index::processCollectionQueues(const CollectionQueue &queue)
{
    kDebug() << "Indexing items in collection" << m_currentCollection;
    Q_FOREACH (const Akonadi::Item &item, queue.unindexQueue) {
        AbstractIndexer *indexer = indexerForItem(item);
        if (indexer) {
            try {
                indexer->remove(item);
            } catch (const Xapian::Error &e) {
                kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
    kDebug() << "\tRemoved:" << queue.unindexQueue.count();

    QMutexLocker locker(&m_queueLock);
    if (m_stop) {
        return;
    }

    QDateTime lastDt = m_lastIndexedItem.isValid() ? m_lastIndexedItem : QDateTime::fromMSecsSinceEpoch(1);

    locker.unlock();

    Q_FOREACH (const FlagsChange &fc, queue.flagsQueue) {
        AbstractIndexer *indexer = indexerForItem(fc.item);
        if (indexer) {
            try {
                indexer->updateFlags(fc.item, fc.addedFlags, fc.removedFlags);
                if (lastDt < fc.item.modificationTime()) {
                    lastDt = fc.item.modificationTime();
                }
            } catch (const Xapian::Error &e) {
                kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
    kDebug() << "\tFlag updates:" << queue.flagsQueue.count();

    locker.relock();
    if (m_stop) {
        return;
    }
    locker.unlock();

    Q_FOREACH (const MoveChange &mc, queue.movesQueue) {
        AbstractIndexer *indexer = indexerForItem(mc.item);
        if (indexer) {
            try {
                indexer->move(mc.item.id(), mc.sourceCollection, mc.destCollection);
                if (lastDt < mc.item.modificationTime()) {
                    lastDt = mc.item.modificationTime();
                }
            } catch (const Xapian::Error &e) {
                kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
    kDebug() << "\tMoves:" << queue.movesQueue.count();

    locker.relock();
    if (m_stop) {
        return;
    }
    locker.unlock();

    Q_FOREACH (const Akonadi::Item &item, queue.indexQueue) {
        AbstractIndexer *indexer = indexerForItem(item);
        if (indexer) {
            try {
                indexer->index(item);
                if (lastDt < item.modificationTime()) {
                    lastDt = item.modificationTime();
                }
            } catch (const Xapian::Error &e) {
                kWarning() << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
    kDebug() << "\tNew/updated:" << queue.indexQueue.count();

    locker.relock();
    if (lastDt  > m_lastIndexedItem) {
        m_lastIndexedItem = lastDt;
    }

    kDebug() << "Done";
}

void Index::processQueue()
{
    kDebug() << "ProcessQueue start";
    QMutexLocker locker(&m_queueLock);

    Q_FOREVER {
        m_running = true;
        locker.unlock();

        processClearCollectionQueue();
        commit();

        locker.relock();
        if (m_stop) {
            break;
        }

        // Take first collection in the queue
        IndexQueue::Iterator iter = m_queue.begin();
        // No collection? Done!
        if (iter == m_queue.end()) {
            break;
        }
        const CollectionQueue queue = *iter;
        // Remove the collection from the queue
        m_currentCollection = iter.key();
        m_queue.erase(iter);
        locker.unlock();

        Q_EMIT currentCollectionChanged(m_currentCollection);

        // Process the collection
        processCollectionQueues(queue);
        commit();

        locker.relock();
        if (m_stop) {
            break;
        }
    }

    kDebug() << "ProcessQueue done!";
    m_currentCollection = -1;
    m_running = false;
    m_queueLock.unlock();

    Q_EMIT currentCollectionChanged(m_currentCollection);
}
