/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2013 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pendingfilequeue.h"

#include <QDebug>

using namespace Baloo;

PendingFileQueue::Entry::Entry(const Baloo::PendingFile& file_, int c)
    : file(file_),
      cnt(c)
{
}

bool PendingFileQueue::Entry::operator==(const Entry& other) const
{
    // we ignore the counter since we need this for the search in queueUrl only
    return file == other.file;
}


PendingFileQueue::PendingFileQueue(QObject* parent)
    : QObject(parent)
{
    // we default to 5 seconds
    m_queueTimeout = 5;
    m_emittedTimeout = 5;

    // setup the timer
    connect(&m_queueTimer, SIGNAL(timeout()),
            this, SLOT(slotTimer()));

    // we check in 1 sec intervals
    m_queueTimer.setInterval(1000);
}

PendingFileQueue::~PendingFileQueue()
{
}

void PendingFileQueue::enqueueUrl(const PendingFile& file)
{
    Entry defaultEntry(file, m_queueTimeout);

    // If the url is already in the queue update its timestamp
    QQueue<Entry>::iterator it = qFind(m_queue.begin(), m_queue.end(), defaultEntry);
    if (it != m_queue.end()) {
        it->cnt = m_queueTimeout;
        it->file.merge(file);
    } else {
        // We check if we just emitted the url, if so we move it to the normal queue
        QHash<QString, int>::iterator iter = m_emittedEntries.find(file.path());
        if (iter != m_emittedEntries.end()) {
            m_queue.enqueue(defaultEntry);
            m_emittedEntries.erase(iter);
        } else {
            // It's not in any of the queues
            defaultEntry.cnt = 0;
            m_queue.enqueue(defaultEntry);
        }
    }

    // make sure the timer is running
    if (!m_queueTimer.isActive()) {
        m_queueTimer.start();
    }

    //
    // The 10 msecs is completely arbitrary. We want to aggregate
    // events instead of instantly emitting timeout as we typically get the
    // same file multiple times but with different flags
    //
    QTimer::singleShot(10, this, SLOT(slotRemoveEmptyEntries()));
}

void PendingFileQueue::setTimeout(int seconds)
{
    m_queueTimeout = seconds;
}

void PendingFileQueue::setWaitTimeout(int seconds)
{
    m_emittedTimeout = seconds;
}

void PendingFileQueue::slotRemoveEmptyEntries()
{
    // we run through the queue, decrease each counter and emit each entry which has a count of 0
    QMutableListIterator<Entry> it(m_queue);
    while (it.hasNext()) {
        Entry& entry = it.next();
        if (entry.cnt <= 0) {
            // Insert into the emitted queue
            m_emittedEntries.insert(entry.file.path(), m_emittedTimeout);

            Q_EMIT urlTimeout(entry.file);
            it.remove();
        }
    }
}

void PendingFileQueue::slotTimer()
{
    // we run through the queue, decrease each counter and emit each entry which has a count of 0
    QMutableListIterator<Entry> it(m_queue);
    while (it.hasNext()) {
        Entry& entry = it.next();
        entry.cnt--;
        if (entry.cnt <= 0) {
            // Insert into the emitted queue
            m_emittedEntries.insert(entry.file.path(), m_emittedTimeout);

            Q_EMIT urlTimeout(entry.file);
            it.remove();
        }
    }

    // Run through all the emitted entires and remove them
    QMutableHashIterator<QString, int> iter(m_emittedEntries);
    while (iter.hasNext()) {
        iter.next();
        iter.value()--;
        if (iter.value() <= 0) {
            iter.remove();
        }
    }

    // stop the timer in case we have nothing left to do
    if (m_queue.isEmpty() && m_emittedEntries.isEmpty()) {
        m_queueTimer.stop();
    }
}
