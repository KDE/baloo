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


#ifndef FILEINDEXINGQUEUE_H
#define FILEINDEXINGQUEUE_H

#include "indexingqueue.h"

#include <KJob>
#include <Soprano/QueryResultIterator>

namespace Nepomuk2
{

class FileIndexingQueue : public IndexingQueue
{
    Q_OBJECT
public:
    explicit FileIndexingQueue(QObject* parent = 0);
    virtual bool isEmpty();
    virtual void fillQueue();

    void clear();
    void clear(const QString& path);
    QUrl currentUrl();

public slots:
    /**
     * Fills up the queue and starts the indexing
     */
    void start();

    void enqueue(const QUrl& url);
signals:
    void beginIndexingFile(const QUrl& url);
    void endIndexingFile(const QUrl& url);

protected:
    virtual void processNextIteration();

private slots:
    void slotFinishedIndexingFile(KJob* job);
    void slotConfigChanged();

private:
    void process(const QUrl& url);

    QQueue<QUrl> m_fileQueue;
    QUrl m_currentUrl;
};
}

#endif // FILEINDEXINGQUEUE_H
