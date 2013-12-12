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


#include "fileindexingqueue.h"
#include "fileindexingjob.h"
#include "fileindexerconfig.h"
#include "util.h"
#include "database.h"

#include <KDebug>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
{
    m_maxSize = 120;
    m_batchSize = 40;

    m_fileQueue.reserve(m_maxSize);

    FileIndexerConfig* config = FileIndexerConfig::self();
    connect(config, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
}

// FIXME: We are not emiting startedIndexing!

void FileIndexingQueue::fillQueue()
{
    if (m_fileQueue.size() >= m_maxSize)
        return;

    Xapian::Enquire enquire(*m_db->xapainDatabase());
    enquire.set_query(Xapian::Query("Z1"));

    Xapian::MSet mset = enquire.get_mset(0, m_maxSize - m_fileQueue.size());
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        FileMapping file(*it);
        if (file.fetch(m_db->sqlDatabase())) {
            m_fileQueue << file;
        }
    }
}

void FileIndexingQueue::enqueue(const FileMapping& file)
{
    if (!m_fileQueue.contains(file)) {
        m_fileQueue.enqueue(file);
        callForNextIteration();
    }
}

bool FileIndexingQueue::isEmpty()
{
    return m_fileQueue.isEmpty();
}

void FileIndexingQueue::processNextIteration()
{
    QList<FileMapping> files;
    for (int i=0; i<m_batchSize && m_fileQueue.size(); i++) {
        files << m_fileQueue.dequeue();
    }

    process(files);
}

void FileIndexingQueue::process(const QList<FileMapping>& files)
{
    FileIndexingJob* job = new FileIndexingJob(files, this);
    connect(job, SIGNAL(deleteDocument(uint)), SIGNAL(deleteDocument(uint)));
    connect(job, SIGNAL(finished(KJob*)), SLOT(slotFinishedIndexingFile(KJob*)));

    job->start();
}

void FileIndexingQueue::slotFinishedIndexingFile(KJob* job)
{
    if (job->error()) {
        kDebug() << job->errorString();
        // FIXME: How do we fix this?
        // updateIndexingLevel(m_db, m_currentFile.id(), 0);
    }

    // The process would have modified the db
    m_db->xapainDatabase()->reopen();
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

void FileIndexingQueue::clear()
{
    m_fileQueue.clear();
}

void FileIndexingQueue::clear(const QString& path)
{
    QMutableListIterator<FileMapping> it(m_fileQueue);
    while (it.hasNext()) {
        if (it.next().url().startsWith(path))
            it.remove();
    }
}

void FileIndexingQueue::slotConfigChanged()
{
    m_fileQueue.clear();
    fillQueue();
}
