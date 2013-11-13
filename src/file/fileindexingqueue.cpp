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
#include <QTimer>

#include <QSqlQuery>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
{
    m_fileQueue.reserve(10);

    FileIndexerConfig* config = FileIndexerConfig::self();
    connect(config, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
}

void FileIndexingQueue::start()
{
    fillQueue();
    Q_EMIT startedIndexing();

    callForNextIteration();
}

void FileIndexingQueue::fillQueue()
{
    /* prevent abuse this API */
    if (m_fileQueue.size() > 0)
        return;

    Xapian::Enquire enquire(*m_db->xapainDatabase());
    enquire.set_query(Xapian::Query("Z1"));

    Xapian::MSet mset = enquire.get_mset(0, 10);
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        int fileId = *it;

        kDebug() << fileId;
        QSqlQuery query(m_db->sqlDatabase());
        query.setForwardOnly(true);
        query.prepare("select url from files where id = ?");
        query.addBindValue(fileId);
        query.exec();

        if (query.next())
            m_fileQueue << query.value(0).toString();
    }
}

void FileIndexingQueue::enqueue(const QString& url)
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
    const QString fileUrl = m_fileQueue.dequeue();
    process(fileUrl);
}

void FileIndexingQueue::process(const QString& url)
{
    m_currentUrl = url;

    KJob* job = new FileIndexingJob(m_db, url);
    job->start();
    Q_EMIT beginIndexingFile(url);
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

    QString url = m_currentUrl;
    m_currentUrl.clear();
    Q_EMIT endIndexingFile(url);
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
    QMutableListIterator<QString> it(m_fileQueue);
    while (it.hasNext()) {
        if (it.next().startsWith(path))
            it.remove();
    }
}


QString FileIndexingQueue::currentUrl()
{
    return m_currentUrl;
}

void FileIndexingQueue::slotConfigChanged()
{
    m_fileQueue.clear();
    fillQueue();
}
