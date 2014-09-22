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


#include <xapian.h>
#include "basicindexingqueue.h"
#include "fileindexerconfig.h"
#include "util.h"
#include "basicindexingjob.h"
#include "database.h"
#include "filemapping.h"

#include "xapiandocument.h"

#include <QDebug>

#include <QDateTime>
#include <QTimer>
#include <QUrl>

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
    QMutableVectorIterator< QPair<FileMapping, UpdateDirFlags> > it(m_paths);
    while (it.hasNext()) {
        it.next();
        if (it.value().first.url().startsWith(path))
            it.remove();
    }
}

bool BasicIndexingQueue::isEmpty()
{
    return m_paths.isEmpty();
}

void BasicIndexingQueue::enqueue(const FileMapping& file)
{
    UpdateDirFlags flags;
    flags |= UpdateRecursive;

    enqueue(file, flags);
}

void BasicIndexingQueue::enqueue(const FileMapping& file, UpdateDirFlags flags)
{
    qDebug() << file.url();
    m_paths.push(qMakePair(file, flags));
    callForNextIteration();
}

void BasicIndexingQueue::processNextIteration()
{
    bool processingFile = false;

    if (!m_paths.isEmpty()) {
        QPair<FileMapping, UpdateDirFlags> pair = m_paths.pop();
        processingFile = process(pair.first, pair.second);
    }

    if (!processingFile)
        finishIteration();
}


bool BasicIndexingQueue::process(FileMapping& file, UpdateDirFlags flags)
{
    bool startedIndexing = false;

    // This mimetype may not be completely accurate, but that's okay. This is
    // just the initial phase of indexing. The second phase can try to find
    // a more accurate mimetype.
    QString mimetype = m_mimeDb.mimeTypeForFile(file.url(), QMimeDatabase::MatchExtension).name();

    bool forced = flags & ForceUpdate;
    bool recursive = flags & UpdateRecursive;
    bool indexingRequired = (flags & ExtendedAttributesOnly) || shouldIndex(file, mimetype);

    QFileInfo info(file.url());
    if (info.isDir()) {
        if (forced || indexingRequired) {
            startedIndexing = true;
            index(file, mimetype, flags);
        }

        // We don't want to follow system links
        if (recursive && !info.isSymLink() && shouldIndexContents(file.url())) {
            QDir::Filters dirFilter = QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs;

            QDirIterator it(file.url(), dirFilter);
            while (it.hasNext()) {
                m_paths.push(qMakePair(FileMapping(it.next()), flags));
            }
        }
    } else if (info.isFile() && (forced || indexingRequired)) {
        startedIndexing = true;
        index(file, mimetype, flags);
    }

    return startedIndexing;
}

bool BasicIndexingQueue::shouldIndex(FileMapping& file, const QString& mimetype) const
{
    bool shouldBeIndexed = m_config->shouldBeIndexed(file.url());
    if (!shouldBeIndexed) {
        return false;
    }

    bool shouldIndexType = m_config->shouldMimeTypeBeIndexed(mimetype);
    if (!shouldIndexType) {
        return false;
    }

    QFileInfo fileInfo(file.url());
    if (!fileInfo.exists()) {
        return false;
    }

    if (!file.fetch(m_db->sqlDatabase())) {
        return true;
    }

    XapianDocument doc = m_db->xapianDatabase()->document(file.id());
    const QString dtStr = doc.fetchTermStartsWith("DT_M");
    if (dtStr.isEmpty()) {
        return true;
    }

    // A folders mtime is updated when a new file is added / removed / renamed
    // we don't really need to reindex a folder when that happens
    // In fact, we never need to reindex a folder
    if (mimetype == QLatin1String("inode/directory")) {
        return false;
    }

    // The 4 is for "DT_M"
    const QDateTime mtime = QDateTime::fromString(dtStr.mid(4), Qt::ISODate);

    if (mtime != fileInfo.lastModified()) {
        return true;
    }

    return false;
}

bool BasicIndexingQueue::shouldIndexContents(const QString& dir)
{
    return m_config->shouldFolderBeIndexed(dir);
}

void BasicIndexingQueue::index(FileMapping& file, const QString& mimetype,
                               UpdateDirFlags flags)
{
    qDebug() << file.id() << file.url();

    bool xattrOnly = (flags & Baloo::ExtendedAttributesOnly);

    if (!xattrOnly) {
        if (file.id() == 0) {
            if (!file.create(m_db->sqlDatabase())) {
                qWarning() << "Cannot create fileMapping for" << file.url();
            }
            else {
                BasicIndexingJob job(file, mimetype);
                if (job.index(m_config->onlyBasicIndexing() ? BasicIndexingJob::CompletedIndexing
                                                            : BasicIndexingJob::OngoingIndexing)) {
                    Q_EMIT newDocument(job.id(), job.document());
                }
            }
        }
    }
    else {
        XapianDocument doc = m_db->xapianDatabase()->document(file.id());

        bool modified = false;
        modified |= doc.removeTermStartsWith("R");
        modified |= doc.removeTermStartsWith("TA");
        modified |= doc.removeTermStartsWith("TAG");
        modified |= doc.removeTermStartsWith("C");

        modified |= BasicIndexingJob::indexXAttr(file.url(), doc);
        if (modified) {
            Q_EMIT newDocument(file.id(), doc.doc());
        }
    }

    QTimer::singleShot(0, this, SLOT(finishIteration()));
}
