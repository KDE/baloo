/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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


#ifndef FILEINDEXER_INDEXINGQUEUE_H
#define FILEINDEXER_INDEXINGQUEUE_H

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QDirIterator>

namespace Baloo
{

/**
 * An abstract class representing the queue. All indexing queues
 * should be derived from this queue.
 *
 */
class IndexingQueue : public QObject
{
    Q_OBJECT
public:
    explicit IndexingQueue(QObject* parent = 0);

    virtual bool isEmpty() = 0;

    /**
     * fill the queue if there is available data, return true if something
     * is enqueued. Should be used in combinition with isEmpty()
     *
     * Default implementation will do nothing.
     */
    virtual void fillQueue();

    /**
     * Sets a artificial delay between each call to processNextIteration.
     * This can be used to slow down the indexing, in order to conserve
     * resources.
     */
    void setDelay(int msec);
    int delay() const;

    bool isSuspended() const;

public Q_SLOTS:
    void suspend();
    void resume();

Q_SIGNALS:
    /**
     * This signal will be emitted when the queue starts
     * processing items
     */
    void startedIndexing();

    /**
     * The signal be emitted once the queue is empty
     */
    void finishedIndexing();

protected:
    /**
     * Process the next iteration in your queue. Once you are done
     * processing the file call finishIndexingFile so that the queue
     * can continue processing and call this function when it is
     * ready for the next iteration.
     *
     * \sa finishIndexingFile
     */
    virtual void processNextIteration() = 0;
    virtual void doSuspend();
    virtual void doResume();
    bool m_sentEvent;
    bool m_suspended;



protected Q_SLOTS:
    /**
     * Call this function when you have finished processing the
     * iteration from processNextIteration.
     *
     * \sa processNextIteration
     */
    void finishIteration();

    void callForNextIteration();

private Q_SLOTS:
    void processNext();

private:
    bool m_shouldEmitStartSignal;
    int m_delay;
};

}


#endif // FILEINDEXER_INDEXINGQUEUE_H
