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
    prepareQueueQuery();
}

void FileIndexingQueue::setMaxSize(int size)
{
    m_maxQueueSize = size;
    prepareQueueQuery();
}

void FileIndexingQueue::setBatchSize(int size)
{
    m_batchSize = size;
}

void FileIndexingQueue::setTestMode(bool mode)
{
    m_testMode = mode;
}

void FileIndexingQueue::prepareQueueQuery()
{
    m_queueQuery.prepare("SELECT url, indexingLevel FROM files WHERE indexingLevel > :levelMin AND indexingLevel < :levelMax LIMIT :limit");
    m_queueQuery.bindValue(":levelMin", SkipIndexing);
    m_queueQuery.bindValue(":levelMax", PendingSave);
    m_queueQuery.bindValue(":limit", m_maxQueueSize);
}

void FileIndexingQueue::fillQueue()
{
    if (!m_queueQuery.isActive()) {
        m_queueQuery.exec();
        if (m_queueQuery.next()) {
            m_queueQuery.seek(-1);
        } else {
            m_queueQuery.finish();
        }
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
            m_extractor->setFollowConfig(false);
        }

        m_extractor->setDatabasePath(m_db->path());
    }

    if (!m_queueQuery.next()) {
        m_queueQuery.finish();
        fillQueue();
        if (!m_queueQuery.isActive()) {
            // we're done! :)
            m_extractor->indexingComplete();
            return;
        }
    }

    m_currentUrl = m_queueQuery.value(0).toString();
    m_extractor->indexFile(m_currentUrl);
}

void FileIndexingQueue::finishedIndexingFile(const QString &url)
{
    Q_UNUSED(url)
    ++m_batchCount;

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
    m_extractor->deleteLater();
    m_extractor = 0;

    finishedIndexingFile(QString());
}

void FileIndexingQueue::dataSaved(const QString &lastUrlSaved)
{
    Q_UNUSED(lastUrlSaved)
    if (!m_queueQuery.isActive()) {
        // indexing is done for now, so lets release the extractor process
        m_extractor->deleteLater();
        m_extractor = 0;
        finishIteration();
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

