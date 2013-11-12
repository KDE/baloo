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

#include <KDebug>
#include <QTimer>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(QObject* parent): IndexingQueue(parent)
{
    m_fileQueue.reserve(10);

    FileIndexerConfig* config = FileIndexerConfig::self();
    connect(config, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
}

void FileIndexingQueue::start()
{
    fillQueue();
    emit startedIndexing();

    callForNextIteration();
}

void FileIndexingQueue::fillQueue()
{
    /* prevent abuse this API */
    if (m_fileQueue.size() > 0)
        return;

    /*
    QString query = QString::fromLatin1("select distinct ?url where { ?r nie:url ?url ; kext:indexingLevel ?l "
                                        " FILTER(?l = 1 ). } LIMIT 10");

    Soprano::Model* model = ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    while (it.next())
        m_fileQueue.enqueue(it[0].uri());*/
}

void FileIndexingQueue::enqueue(const QUrl& url)
{
    if (!m_fileQueue.contains(url)) {
        m_fileQueue.enqueue(url);
        callForNextIteration();
    }
}


bool FileIndexingQueue::isEmpty()
{
    return m_fileQueue.isEmpty();
}

void FileIndexingQueue::processNextIteration()
{
    const QUrl fileUrl = m_fileQueue.dequeue();
    process(fileUrl);
}

void FileIndexingQueue::process(const QUrl& url)
{
    m_currentUrl = url;

    KJob* job = new FileIndexingJob(url);
    job->start();
    emit beginIndexingFile(url);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinishedIndexingFile(KJob*)));
}

void FileIndexingQueue::slotFinishedIndexingFile(KJob* job)
{
    if (job->error()) {
        kDebug() << job->errorString();
        /*// Get the uri of the current file
        QString query = QString::fromLatin1("select ?r where { ?r nie:url %1 . }")
                        .arg(Soprano::Node::resourceToN3(m_currentUrl));
        Soprano::Model* model = ResourceManager::instance()->mainModel();
        Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);

        if (it.next()) {
            // Update the indexing level to -1, signalling an error,
            // so the next round of the queue doesn't try to index it again.
            updateIndexingLevel(it[0].uri(), -1);
        }*/
    }

    QUrl url = m_currentUrl;
    m_currentUrl.clear();
    emit endIndexingFile(url);
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

void FileIndexingQueue::clear()
{
    m_currentUrl.clear();
    m_fileQueue.clear();
}

void FileIndexingQueue::clear(const QString& path)
{
    QMutableListIterator<QUrl> it(m_fileQueue);
    while (it.hasNext()) {
        if (it.next().toLocalFile().startsWith(path))
            it.remove();
    }
}


QUrl FileIndexingQueue::currentUrl()
{
    return m_currentUrl;
}

void FileIndexingQueue::slotConfigChanged()
{
    m_fileQueue.clear();
    fillQueue();
}
