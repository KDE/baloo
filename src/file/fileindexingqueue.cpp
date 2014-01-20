/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012-2013  Vishesh Handa <me@vhanda.in>
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


#include "fileindexingqueue.h"
#include "fileindexingjob.h"
#include "util.h"
#include "database.h"

#include <KDebug>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
{
    m_maxSize = 1200;
    m_batchSize = 40;

    m_fileQueue.reserve(m_maxSize);
}

// FIXME: We are not emiting startedIndexing!

void FileIndexingQueue::fillQueue()
{
    if (m_fileQueue.size() >= m_maxSize)
        return;

    Xapian::Enquire enquire(*m_db->xapianDatabase());
    enquire.set_query(Xapian::Query("Z1"));

    m_db->xapianDatabase()->reopen();
    Xapian::MSet mset = enquire.get_mset(0, m_maxSize - m_fileQueue.size());
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        m_fileQueue << *it;
    }
}

void FileIndexingQueue::enqueue(const FileMapping& file)
{
    if (!m_fileQueue.contains(file.id())) {
        m_fileQueue << file.id();
        callForNextIteration();
    }
}

bool FileIndexingQueue::isEmpty()
{
    return m_fileQueue.isEmpty();
}

void FileIndexingQueue::processNextIteration()
{
    QVector<uint> files;
    files.reserve(m_batchSize);

    for (int i=0; i<m_batchSize && m_fileQueue.size(); i++) {
        files << m_fileQueue.pop();
    }

    process(files);
}

void FileIndexingQueue::process(const QVector<uint>& files)
{
    FileIndexingJob* job = new FileIndexingJob(files, this);
    connect(job, SIGNAL(indexingFailed(uint)), this, SLOT(slotIndexingFailed(uint)));
    connect(job, SIGNAL(finished(KJob*)), SLOT(slotFinishedIndexingFile(KJob*)));

    job->start();
}

void FileIndexingQueue::slotFinishedIndexingFile(KJob*)
{
    // The process would have modified the db
    m_db->xapianDatabase()->reopen();
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

void FileIndexingQueue::slotIndexingFailed(uint id)
{
    m_db->xapianDatabase()->reopen();
    Xapian::Document doc = m_db->xapianDatabase()->get_document(id);

    updateIndexingLevel(doc, -1);
    Q_EMIT newDocument(id, doc);
}


void FileIndexingQueue::clear()
{
    m_fileQueue.clear();
}
