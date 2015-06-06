/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012-2015  Vishesh Handa <vhanda@kde.org>
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
#include "database.h"
#include "transaction.h"

#include <QStandardPaths>
#include <QDebug>

using namespace Baloo;

FileIndexingQueue::FileIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
    , m_extractorIdle(true)
    , m_extractorProcess(0)
    , m_extractorPath(QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor")))
    , m_timeCurrentFile(this)
{
    m_maxSize = 1200;
    m_batchSize = 40;

    m_fileQueue.reserve(m_maxSize);

    // Time limit per file TODO: figure out appropriate limit
    m_processTimeout = 5 * 60 * 1000;
    m_timeCurrentFile.setSingleShot(true);

    connect(&m_timeCurrentFile, &QTimer::timeout, this, &FileIndexingQueue::slotHandleProcessTimeout);

    startExtractorProcess();
}

FileIndexingQueue::~FileIndexingQueue()
{
    stopExtractorProcess();
}


void FileIndexingQueue::startExtractorProcess()
{
    m_extractorProcess = new QProcess(this);
    connect(m_extractorProcess, &QProcess::readyRead, this, &FileIndexingQueue::slotFileIndexed);

    m_extractorProcess->start(m_extractorPath, QStringList(), QIODevice::Unbuffered | QIODevice::ReadWrite);
    m_extractorProcess->setReadChannel(QProcess::StandardOutput);
}

void FileIndexingQueue::stopExtractorProcess()
{
    m_extractorProcess->disconnect(this);
    m_extractorProcess->deleteLater();
    m_extractorProcess = 0;
}


void FileIndexingQueue::fillQueue()
{
    // we do not refil queue unless it is empty to avoid duplicates
    if (m_fileQueue.size() >= m_maxSize || !m_extractorIdle)
        return;

    QVector<quint64> newItems;
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        newItems = tr.fetchPhaseOneIds(m_maxSize - m_fileQueue.size());
    }
    m_fileQueue << newItems;
}

bool FileIndexingQueue::isEmpty()
{
    return m_fileQueue.isEmpty();
}

void FileIndexingQueue::processNextIteration()
{
    Q_ASSERT(m_extractorProcess->state() == QProcess::Running);
    Q_ASSERT(m_extractorIdle);

    //m_timeCurrentFile.start(m_processTimeout);

    QVector<quint64> ids;
    for (int i = 0; i < m_batchSize && m_fileQueue.size(); i++) {
        ids << m_fileQueue.pop();
    }

    quint32 size = ids.size();
    QByteArray batchData;
    batchData.append(reinterpret_cast<char*>(&size), sizeof(quint32));
    for (quint64 id : ids) {
        batchData.append(reinterpret_cast<char*>(&id), sizeof(quint64));
    }
    m_extractorIdle = false;
    m_extractorProcess->write(batchData.data(), batchData.size());
}

void FileIndexingQueue::slotHandleProcessTimeout()
{
    //TODO
    /*qDebug() << "Process took too long killing";
    stopExtractorProcess();
    indexingFailed();
    startExtractorProcess();
    // Failed file has already been removed from queue, go back to indexing from the next file
    processNextIteration();*/
}

void FileIndexingQueue::indexingFailed()
{
    qDebug() << "Timeout with docid: " << m_currentFile;
    /*
     * FIXME: Handle indexing failed
     * Ideally, update the indexing level!
     * NOTE: failed file id will be in m_currentFile
     */
}


void FileIndexingQueue::clear()
{
    m_fileQueue.clear();
}


void FileIndexingQueue::slotFileIndexed()
{
    //m_timeCurrentFile.stop();

    QByteArray output = m_extractorProcess->readAll();

    m_extractorIdle = true;
    if (m_fileQueue.isEmpty()) {
        fillQueue();
    }
    finishIteration();
}

