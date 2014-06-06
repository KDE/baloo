/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "indexingqueue.h"

#include <QtCore/QTimer>
#include <QDebug>

using namespace Baloo;

IndexingQueue::IndexingQueue(QObject* parent): QObject(parent)
{
    m_sentEvent = false;
    m_suspended = false;
    m_shouldEmitStartSignal = true;
    m_delay = 0;
}

void IndexingQueue::fillQueue()
{
}

void IndexingQueue::processNext()
{
    if (m_suspended || isEmpty()) {
        m_sentEvent = false;
        return;
    }

    processNextIteration();
}


void IndexingQueue::resume()
{
    doResume();
    m_suspended = false;
    if (isEmpty())
        fillQueue();
    if (!isEmpty())
        callForNextIteration();
}

void IndexingQueue::suspend()
{
    doSuspend();
    m_suspended = true;
}

void IndexingQueue::callForNextIteration()
{
    // If already called callForNextIteration
    if (m_sentEvent)
        return;

    if (isEmpty()) {
        Q_EMIT finishedIndexing();
        m_shouldEmitStartSignal = true;
        return;
    }
    else if (m_shouldEmitStartSignal) {
        Q_EMIT startedIndexing();
        m_shouldEmitStartSignal = false;
    }

    if (!m_suspended) {
        QTimer::singleShot(m_delay, this, SLOT(processNext()));
        m_sentEvent = true;
    }
}

void IndexingQueue::finishIteration()
{
    if (m_sentEvent) {
        m_sentEvent = false;
        callForNextIteration();
    }
}

void IndexingQueue::setDelay(int msec)
{
    m_delay = msec;
}

void IndexingQueue::doResume()
{
}

void IndexingQueue::doSuspend()
{
}

bool IndexingQueue::isSuspended() const
{
    return m_suspended;
}
