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

#include <QStandardPaths>
#include <QDebug>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
    , m_testMode(false)
    , m_indexJob(0)
{
    m_maxSize = 1200;
    m_batchSize = 40;

    m_fileQueue.reserve(m_maxSize);
}

void FileIndexingQueue::fillQueue()
{
    if (m_fileQueue.size() >= m_maxSize) {
        return;
    }

    // We do not want to refill the queue when a job is going on
    // this will result in unnecessary duplicates
    if (m_indexJob) {
        return;
    }

    try {
        Xapian::Database* db = m_db->xapianDatabase()->db();
        Xapian::Enquire enquire(*db);
        enquire.set_query(Xapian::Query("Z1"));
        enquire.set_weighting_scheme(Xapian::BoolWeight());

        Xapian::MSet mset = enquire.get_mset(0, m_maxSize - m_fileQueue.size());
        Xapian::MSetIterator it = mset.begin();
        for (; it != mset.end(); ++it) {
            m_fileQueue << *it;
        }
    }
    catch (const Xapian::DatabaseModifiedError&) {
        fillQueue();
    }
    catch (const Xapian::Error&) {
        return;
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

    for (int i=0; i<m_batchSize && m_fileQueue.size(); ++i) {
        files << m_fileQueue.pop();
    }

    Q_ASSERT(m_indexJob == 0);
    m_indexJob = new FileIndexingJob(files, this);
    if (m_testMode) {
        m_indexJob->setCustomDbPath(m_db->path());
    }
    connect(m_indexJob, SIGNAL(indexingFailed(uint)), this, SLOT(slotIndexingFailed(uint)));
    connect(m_indexJob, SIGNAL(finished(KJob*)), SLOT(slotFinishedIndexingFile(KJob*)));

    m_indexJob->start();
}

void FileIndexingQueue::slotFinishedIndexingFile(KJob* job)
{
    Q_ASSERT(job == m_indexJob);
    m_indexJob = 0;

    // The process would have modified the db
    m_db->xapianDatabase()->db()->reopen();
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

void FileIndexingQueue::slotIndexingFailed(uint id)
{
    m_db->xapianDatabase()->db()->reopen();
    Xapian::Document doc;
    try {
        Xapian::Document doc = m_db->xapianDatabase()->db()->get_document(id);
        updateIndexingLevel(doc, SkipIndexing);
        Q_EMIT newDocument(id, doc);
    } catch (const Xapian::Error& err) {
    }
}


void FileIndexingQueue::clear()
{
    m_fileQueue.clear();
}

void FileIndexingQueue::doResume()
{
    if (m_indexJob) {
        m_indexJob->resume();
    }
}

void FileIndexingQueue::doSuspend()
{
    if (m_indexJob) {
        m_indexJob->suspend();
    }
}

