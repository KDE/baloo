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
#include "filemapping.h"
#include <QStandardPaths>
#include <QDebug>

#include "lucenedocument.h"

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(LuceneIndex *index, QObject* parent)
    : IndexingQueue(parent)
    , m_index(index)
    , m_testMode(false)
    , m_indexJob(0)
{
    m_maxSize = 1200;
    m_batchSize = 40;
    m_reader = index->IndexReader();
    m_fileQueue.reserve(m_maxSize);
}

void FileIndexingQueue::fillQueue()
{
    if (m_fileQueue.size() >= m_maxSize)
        return;

    // We do not want to refill the queue when a job is going on
    // this will result in unnecessary duplicates
    if (m_indexJob)
        return;
    Lucene::SearcherPtr searcher = Lucene::newLucene<Lucene::IndexSearcher>(m_reader);
    Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(L"Z", L"1");
    Lucene::TermQueryPtr query = Lucene::newLucene<Lucene::TermQuery>(term);
    Lucene::TopDocsPtr topdocs = searcher->search(query, m_maxSize - m_fileQueue.size());
    Lucene::Collection<Lucene::ScoreDocPtr> scoreDocs = topdocs->scoreDocs;
    Lucene::Collection<Lucene::ScoreDocPtr>::iterator it = scoreDocs.begin();
    for (; it != scoreDocs.end(); ++it) {
        LuceneDocument doc(m_reader->document(*it->doc));
        m_fileQueue << doc.getFieldValues("URL").at(0);
    }
}

bool FileIndexingQueue::isEmpty()
{
    return m_fileQueue.isEmpty();
}

void FileIndexingQueue::processNextIteration()
{
    QStringList files;
    files.reserve(m_batchSize);

    for (int i=0; i<m_batchSize && m_fileQueue.size(); ++i) {
        files << m_fileQueue.pop();
    }

    Q_ASSERT(m_indexJob == 0);
    m_indexJob = new FileIndexingJob(files, this);
    if (m_testMode) {
        m_indexJob->setCustomDbPath(m_testModePath);
    }
    connect(m_indexJob, &FileIndexingJob::indexingFailed, this, &FileIndexingQueue::slotIndexingFailed);
    connect(m_indexJob, &FileIndexingJob::finished, this, &FileIndexingQueue::slotFinishedIndexingFile);

    m_indexJob->start();
}

void FileIndexingQueue::slotFinishedIndexingFile(KJob* job)
{
    Q_ASSERT(job == m_indexJob);
    m_indexJob = 0;

    // The process would have modified the db
    m_reader->reopen();
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

void FileIndexingQueue::slotIndexingFailed(QString& path)
{
    m_reader->reopen();
    LuceneDocument doc;
    FileMapping map(path);
    map.fetch(m_reader);
    LuceneDocument doc(m_reader->document(map.id()));
    doc.removeFields("Z");
    doc.addIndexedField("Z", "-1");
    Q_EMIT newDocument(map.id(), doc);
}


void FileIndexingQueue::clear()
{
    m_fileQueue.clear();
}

void FileIndexingQueue::doResume()
{
    if (m_indexJob)
        m_indexJob->resume();
}

void FileIndexingQueue::doSuspend()
{
    if (m_indexJob)
        m_indexJob->suspend();
}

