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

#ifndef PENDINGFILEQUEUE_H
#define PENDINGFILEQUEUE_H

#include "pendingfile.h"

#include <QObject>
#include <QString>
#include <QHash>
#include <QTimer>
#include <QVector>

namespace Baloo {

/**
 *
 */
class PendingFileQueue : public QObject
{
    Q_OBJECT

public:
    explicit PendingFileQueue(QObject* parent = 0);
    ~PendingFileQueue();

Q_SIGNALS:
    void indexNewFile(const QString& fileUrl);
    void indexModifiedFile(const QString& fileUrl);
    void indexXAttr(const QString& fileUrl);
    void removeFileIndex(const QString& fileUrl);

public Q_SLOTS:
    void enqueue(const PendingFile& file);

    /**
     * The number of seconds the file should be tracked after it has
     * been emitted. This defaults to 2 minutes
     */
    void setTrackingTime(int seconds);

    /**
     * Set the minimum amount of seconds a file should be kept in the queue
     * on receiving successive modifications events.
     */
    void setMinimumTimeout(int seconds);
    void setMaximumTimeout(int seconds);

private Q_SLOTS:
    void processCache();
    void processPendingFiles();
    void clearRecentlyEmitted();

private:
    QVector<PendingFile> m_cache;

    QTimer m_cacheTimer;
    QTimer m_clearRecentlyEmittedTimer;
    QTimer m_pendingFilesTimer;

    /**
     * Holds the list of files that were recently emitted along with
     * the time they were last emitted.
     */
    QHash<QString, QTime> m_recentlyEmitted;

    /**
     * The QTime contains the time when these file events should be processed.
     */
    QHash<QString, QTime> m_pendingFiles;

    int m_minTimeout;
    int m_maxTimeout;
    int m_trackingTime;
};

}

#endif
