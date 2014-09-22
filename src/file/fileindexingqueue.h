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
#include <QSqlQuery>
#include <KJob>

class Database;

namespace Baloo
{
class ExtractorClient;

class FileIndexingQueue : public IndexingQueue
{
    Q_OBJECT
public:
    FileIndexingQueue(Database* db, QObject* parent = 0);
    virtual bool isEmpty();
    virtual void fillQueue();

    void clear();

    void setMaxSize(int size) { m_maxQueueSize = size; }
    void setBatchSize(int size) { m_batchSize = size; }
    void setTestMode(bool mode) { m_testMode = mode; }

Q_SIGNALS:
    void newDocument(uint docid, const Xapian::Document& doc);

protected:
    virtual void processNextIteration();
    virtual void doSuspend();
    virtual void doResume();

private Q_SLOTS:
    void finishedIndexingFile(const QString &url);
    void indexingFailed();
    void dataSaved(const QString &url);

private:
    QStack<uint> m_fileQueue;
    Database* m_db;
    QSqlQuery m_queueQuery;
    QStringList m_pendingSave;
    QString m_currentUrl;

    bool m_testMode;
    bool m_suspended;
    int m_batchCount;
    int m_batchSize;
    int m_maxQueueSize;

    ExtractorClient *m_extractor;
};
}

#endif // FILEINDEXINGQUEUE_H
