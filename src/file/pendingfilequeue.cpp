/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2013-2014 Vishesh Handa <me@vhanda.in>

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
#include <QDateTime>

using namespace Baloo;

PendingFileQueue::PendingFileQueue(QObject* parent)
    : QObject(parent)
{
    m_cacheTimer.setInterval(10);
    m_cacheTimer.setSingleShot(true);
    connect(&m_cacheTimer, &QTimer::timeout, this, &PendingFileQueue::processCache);

    m_trackingTime = 120;

    m_clearRecentlyEmittedTimer.setInterval(m_trackingTime * 1000);
    m_clearRecentlyEmittedTimer.setSingleShot(true);
    connect(&m_clearRecentlyEmittedTimer, &QTimer::timeout,
            this, &PendingFileQueue::clearRecentlyEmitted);

    m_minTimeout = 5;
    m_maxTimeout = 60;
    m_pendingFilesTimer.setInterval(m_minTimeout);
    m_pendingFilesTimer.setSingleShot(true);
    connect(&m_pendingFilesTimer, &QTimer::timeout,
            this, &PendingFileQueue::processPendingFiles);
}

PendingFileQueue::~PendingFileQueue()
{
}

void PendingFileQueue::enqueue(const PendingFile& file)
{
    int i = m_cache.indexOf(file);
    if (i == -1) {
        m_cache << file;
    } else {
        m_cache[i].merge(file);
    }

    m_cacheTimer.start();
}

void PendingFileQueue::processCache()
{
    QTime currentTime = QTime::currentTime();

    for (const PendingFile& file : m_cache) {
        if (file.shouldRemoveIndex()) {
            Q_EMIT removeFileIndex(file.path());

            m_recentlyEmitted.remove(file.path());
            m_pendingFiles.remove(file.path());
        }
        else if (file.shouldIndexXAttrOnly()) {
            Q_EMIT indexXAttr(file.path());
        }
        else if (file.shouldIndexContents()) {
            if (m_pendingFiles.contains(file.path())) {
                QTime time = m_pendingFiles[file.path()];

                int secondsLeft = currentTime.secsTo(time);
                secondsLeft = qBound(m_minTimeout, secondsLeft * 2, m_maxTimeout);

                time = currentTime.addSecs(secondsLeft);
                m_pendingFiles[file.path()] = time;
            }
            else if (m_recentlyEmitted.contains(file.path())) {
                QTime time = currentTime.addSecs(m_minTimeout);
                m_pendingFiles[file.path()] = time;
            }
            else {
                Q_EMIT indexFile(file.path());
                m_recentlyEmitted.insert(file.path(), currentTime);
            }
        } else {
            Q_ASSERT_X(false, "FileWatch", "The PendingFile should always have some flags set");
        }
    }

    m_cache.clear();

    if (!m_pendingFiles.isEmpty() && !m_pendingFilesTimer.isActive()) {
        m_pendingFilesTimer.setInterval(m_minTimeout * 1000);
        m_pendingFilesTimer.start();
    }

    if (!m_recentlyEmitted.isEmpty() && !m_clearRecentlyEmittedTimer.isActive()) {
        m_clearRecentlyEmittedTimer.setInterval(m_trackingTime * 1000);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::clearRecentlyEmitted()
{
    QTime time = QTime::currentTime();
    int nextUpdate = m_trackingTime;

    QMutableHashIterator<QString, QTime> it(m_recentlyEmitted);
    while (it.hasNext()) {
        it.next();

        int secondsSinceEmitted = it.value().secsTo(time);
        if (secondsSinceEmitted >= m_trackingTime) {
            it.remove();
        } else {
            int timeLeft = m_trackingTime - secondsSinceEmitted;
            nextUpdate = qMin(nextUpdate, timeLeft);
        }
    }

    if (!m_recentlyEmitted.isEmpty()) {
        m_clearRecentlyEmittedTimer.setInterval(nextUpdate * 1000);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::processPendingFiles()
{
    QTime currentTime = QTime::currentTime();
    int nextUpdate = m_maxTimeout;

    QMutableHashIterator<QString, QTime> it(m_pendingFiles);
    while (it.hasNext()) {
        it.next();

        int secondsLeft = currentTime.secsTo(it.value());
        if (secondsLeft <= 0) {
            Q_EMIT indexFile(it.key());
            m_recentlyEmitted.insert(it.key(), currentTime);

            it.remove();
        }
        else {
            nextUpdate = qMin(secondsLeft, nextUpdate);
        }
    }

    if (!m_pendingFiles.isEmpty()) {
        m_pendingFilesTimer.setInterval(nextUpdate * 1000);
        m_pendingFilesTimer.start();
    }

    if (!m_recentlyEmitted.isEmpty() && !m_clearRecentlyEmittedTimer.isActive()) {
        m_clearRecentlyEmittedTimer.setInterval(m_trackingTime * 1000);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::setTrackingTime(int seconds)
{
    m_trackingTime = seconds;
}

void PendingFileQueue::setMinimumTimeout(int seconds)
{
    m_minTimeout = seconds;
}

void PendingFileQueue::setMaximumTimeout(int seconds)
{
    m_maxTimeout = seconds;
}
