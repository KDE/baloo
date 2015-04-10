/*
    This file is part of the KDE Baloo project.
    Copyright (C) 2012-2015  Vishesh Handa <vhanda@kde.org>

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


#include "basicindexingqueue.h"
#include "fileindexerconfig.h"
#include "basicindexingjob.h"
#include "database.h"
#include "transaction.h"
#include "unindexedfileiterator.h"
#include "idutils.h"

#include <QDebug>
#include <QDateTime>
#include <QTimer>

using namespace Baloo;

BasicIndexingQueue::BasicIndexingQueue(Database* db, FileIndexerConfig* config, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
    , m_config(config)
{
}

void BasicIndexingQueue::clear()
{
    m_paths.clear();
}

void BasicIndexingQueue::clear(const QString& path)
{
    QMutableVectorIterator< QPair<QString, UpdateDirFlags> > it(m_paths);
    while (it.hasNext()) {
        it.next();
        if (it.value().first.startsWith(path))
            it.remove();
    }
}

bool BasicIndexingQueue::isEmpty()
{
    return m_paths.isEmpty();
}

void BasicIndexingQueue::enqueue(const QString& file, UpdateDirFlags flags)
{
    qDebug() << file;
    m_paths.push(qMakePair(file, flags));
    callForNextIteration();
}

void BasicIndexingQueue::processNextIteration()
{
    bool processingFile = false;

    if (!m_paths.isEmpty()) {
        QPair<QString, UpdateDirFlags> pair = m_paths.pop();
        processingFile = process(pair.first, pair.second);
    }

    if (!processingFile)
        finishIteration();
}


bool BasicIndexingQueue::process(const QString& file, UpdateDirFlags flags)
{
    bool startedIndexing = false;

    // FIXME: forced flags and XAttr only!
    //bool forced = flags & ForceUpdate;
    //bool indexingRequired = (flags & ExtendedAttributesOnly);// || shouldIndex(file, mimetype);

    Transaction tr(m_db, Transaction::ReadWrite);

    // FIXME: Does this take files?
    UnIndexedFileIterator it(m_config, &tr, file);
    while (!it.next().isEmpty()) {
        index(&tr, it.filePath(), it.mimetype(), flags);
        startedIndexing = true;
    }

    if (startedIndexing) {
        tr.commit();
    } else {
        tr.abort();
    }

    return startedIndexing;
}

void BasicIndexingQueue::index(Transaction* tr, const QString& file, const QString& mimetype,
                               UpdateDirFlags flags)
{
    Q_ASSERT(tr);

    bool xattrOnly = (flags & Baloo::ExtendedAttributesOnly);
    bool newDoc = !tr->hasDocument(filePathToId(QFile::encodeName(file)));

    if (newDoc) {
        BasicIndexingJob job(file, mimetype, m_config->onlyBasicIndexing());
        job.index();

        tr->addDocument(job.document());
    }

    else if (!xattrOnly) {
        BasicIndexingJob job(file, mimetype, m_config->onlyBasicIndexing());
        if (job.index()) {
            tr->replaceDocument(job.document(), Transaction::DocumentTime);
            tr->setPhaseOne(job.document().id());
        }
    }
    else {
        BasicIndexingJob job(file, mimetype, m_config->onlyBasicIndexing());
        if (job.index()) {
            tr->replaceDocument(job.document(), Transaction::XAttrTerms);
        }
    }

    QTimer::singleShot(0, this, SLOT(finishIteration()));
}
