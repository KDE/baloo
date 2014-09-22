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
#include "util.h"
#include "database.h"
#include "lib/extractorclient.h"

#include <QStandardPaths>
#include <QDebug>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
    , m_queueQuery(m_db->sqlDatabase())
    , m_testMode(false)
    , m_suspended(false)
    , m_batchCount(0)
    , m_batchSize(40)
    , m_maxQueueSize(1200)
    , m_extractor(0)
{
    m_queueQuery.prepare("SELECT url FROM files WHERE indexingLevel > ? AND indexingLevel < ? LIMIT ?");
    m_queueQuery.addBindValue(SkipIndexing);
    m_queueQuery.addBindValue(CompletelyIndexed);
    m_queueQuery.addBindValue(m_maxQueueSize);
}

void FileIndexingQueue::fillQueue()
{
    if (!m_queueQuery.isActive()) {
        m_queueQuery.exec();
    }
}

bool FileIndexingQueue::isEmpty()
{
    return !m_queueQuery.isActive();
}

void FileIndexingQueue::processNextIteration()
{
    if (!m_extractor) {
        m_extractor = new ExtractorClient;
        QObject::connect(m_extractor, &Baloo::ExtractorClient::extractorDied, this, &FileIndexingQueue::indexingFailed);
        QObject::connect(m_extractor, &Baloo::ExtractorClient::fileIndexed, this, &FileIndexingQueue::finishedIndexingFile);
        QObject::connect(m_extractor, &Baloo::ExtractorClient::dataSaved, this, &FileIndexingQueue::dataSaved);

        if (m_testMode) {
            m_extractor->enableDebuging(true);
            m_extractor->setDatabasePath(m_db->path());
        }
    }

    if (!m_queueQuery.next()) {
        m_queueQuery.finish();
        fillQueue();
        if (!m_queueQuery.next()) {
            // still nothing.. we're done! :)
            m_queueQuery.finish();
            m_extractor->indexingComplete();
            finishIteration();
            return;
        }
    }

    m_currentUrl = m_queueQuery.value(0).toString();
    m_extractor->indexFile(m_currentUrl);
}

void FileIndexingQueue::finishedIndexingFile(const QString &url)
{
    ++m_batchCount;
    if (!url.isEmpty()) {
        m_pendingSave << url;
    }

    if (m_batchCount >= m_batchSize) {
        m_batchCount = 0;
        finishIteration();
    } else if (!m_suspended) {
        processNextIteration();
    }
}

void FileIndexingQueue::indexingFailed()
{
    updateIndexingLevel(m_currentUrl, SkipIndexing, m_db->sqlDatabase());
    delete m_extractor;
    m_extractor = 0;

    finishedIndexingFile(QString());
}

void FileIndexingQueue::dataSaved(const QString &lastUrlSaved)
{
    QStringListIterator it(m_pendingSave);
    while (it.hasNext()) {
        const QString url = it.next();
        updateIndexingLevel(url, CompletelyIndexed, m_db->sqlDatabase());

        if (url == lastUrlSaved) {
            break;
        }
    }

    if (it.hasNext()) {
        // incomplete save; we still are waiting on urls to be saved
        QStringList remainder;
        while (it.hasNext()) {
            remainder << it.next();
        }

        m_pendingSave = remainder;
    } else {
        // we got them all... move on
        m_pendingSave.clear();

        if (!m_queueQuery.isActive()) {
            // indexing is done for now, so lets release the extractor process
            delete m_extractor;
            m_extractor = 0;
        }
    }
}

void FileIndexingQueue::clear()
{
    m_queueQuery.finish();
}

void FileIndexingQueue::doResume()
{
    m_suspended = false;
    processNextIteration();
}

void FileIndexingQueue::doSuspend()
{
    m_suspended = true;
}

