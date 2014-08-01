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

namespace Baloo {

/**
 * The active file queue maintains a queue of file paths
 * with a timestamp.
 *
 * When a file enters the queue, it will immediately time out.
 * If the file has just timed out (default is upto 5 seconds), then it will
 * time out after a given time. As soon as a file is queued again its timestamp
 * will be reset and the timing restarts.
 *
 * This allows to "compress" file modification events of downloads
 * and the like into a single event resulting in a smoother
 * experience for the user.
 *
 * Whereas modification events which occur only one will immediately
 * be sent forward.
 *
 * \author Sebastian Trueg <trueg@ĸde.org>
 */
class PendingFileQueue : public QObject
{
    Q_OBJECT

public:
    PendingFileQueue(QObject* parent = 0);
    ~PendingFileQueue();

Q_SIGNALS:
    void urlTimeout(const PendingFile& file);

public Q_SLOTS:
    void enqueueUrl(const PendingFile& file);

    /**
     * Set the timeout in seconds. Be aware that the timeout
     * will not be exact. For internal reasons the queue tries
     * to roughly match the configured timeout. It only guarantees
     * that the timeout will be between \p seconds and \p seconds+1.
     */
    void setTimeout(int seconds);

    /**
     * Sets the timeout in seconds, that the queue should track the url
     * once it has already been emitted.
     */
    void setWaitTimeout(int seconds);

private Q_SLOTS:
    void slotTimer();
    void slotRemoveEmptyEntries();

private:
    class Private;
    Private* const d;
};

}

#endif
