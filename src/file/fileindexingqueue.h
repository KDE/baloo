/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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


#ifndef FILEINDEXINGQUEUE_H
#define FILEINDEXINGQUEUE_H

#include "indexingqueue.h"
#include "filemapping.h"
#include <xapian.h>

#include <QStack>
#include <KJob>

class Database;

namespace Baloo
{
class FileIndexingJob;

class FileIndexingQueue : public IndexingQueue
{
    Q_OBJECT
public:
    FileIndexingQueue(Database* db, QObject* parent = 0);
    virtual bool isEmpty();
    virtual void fillQueue();

    void clear();

Q_SIGNALS:
    void newDocument(uint docid, const Xapian::Document& doc);
    void deleteDocument(unsigned docid);

protected:
    virtual void processNextIteration();
    virtual void doSuspend();
    virtual void doResume();

private Q_SLOTS:
    void slotFinishedIndexingFile(KJob* job);
    void slotIndexingFailed(uint doc);

private:
    QStack<uint> m_fileQueue;
    Database* m_db;

    int m_maxSize;
    int m_batchSize;

    FileIndexingJob* m_indexJob;
};
}

#endif // FILEINDEXINGQUEUE_H
