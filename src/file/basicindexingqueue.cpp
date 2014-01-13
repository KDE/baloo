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

#include <KDebug>
#include <KMimeType>

#include <QDateTime>
#include <QTimer>

using namespace Baloo;

BasicIndexingQueue::BasicIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
{

}

void BasicIndexingQueue::clear()
{
    m_currentFile.clear();
    m_currentFlags = NoUpdateFlags;
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

QString BasicIndexingQueue::currentUrl() const
{
    return m_currentFile.url();
}

UpdateDirFlags BasicIndexingQueue::currentFlags() const
{
    return m_currentFlags;
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
    kDebug() << file.url();
    m_paths.push(qMakePair(file, flags));
    callForNextIteration();

    Q_EMIT startedIndexing();
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

    QString mimetype = KMimeType::findByUrl(QUrl::fromLocalFile(file.url()))->name();

    bool forced = flags & ForceUpdate;
    bool recursive = flags & UpdateRecursive;
    bool indexingRequired = shouldIndex(file, mimetype);

    QFileInfo info(file.url());
    if (info.isDir()) {
        if (forced || indexingRequired) {
            m_currentFile = file;
            m_currentFlags = flags;
            m_currentMimeType = mimetype;

            startedIndexing = true;
            index(file);
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
        m_currentFile = file;
        m_currentFlags = flags;
        m_currentMimeType = mimetype;

        startedIndexing = true;
        index(file);
    }

    return startedIndexing;
}

bool BasicIndexingQueue::shouldIndex(FileMapping& file, const QString& mimetype) const
{
    bool shouldBeIndexed = FileIndexerConfig::self()->shouldBeIndexed(file.url());
    if (!shouldBeIndexed)
        return false;

    bool shouldIndexType = FileIndexerConfig::self()->shouldMimeTypeBeIndexed(mimetype);
    if (!shouldIndexType)
        return false;

    QFileInfo fileInfo(file.url());
    if (!fileInfo.exists())
        return false;

    if (!file.fetch(m_db->sqlDatabase())) {
        return true;
    }

    try {
        reopenIfRequired(m_db->xapianDatabase());
        Xapian::Document doc = m_db->xapianDatabase()->get_document(file.id());
        Xapian::TermIterator it = doc.termlist_begin();
        it.skip_to("DT_M");
        if (it == doc.termlist_end()) {
            return true;
        }

        // A folders mtime is updated when a new file is added / removed / renamed
        // we don't really need to reindex a folder when that happens
        // In fact, we never need to reindex a folder
        if (mimetype == QLatin1String("inode/directory"))
            return false;

        // The 4 is for "DT_M"
        const QString str = QString::fromStdString(*it).mid(4);
        const QDateTime mtime = QDateTime::fromString(str, Qt::ISODate);

        if (mtime != fileInfo.lastModified())
            return true;
    }
    catch (const Xapian::DocNotFoundError&) {
        return true;
    }

    return false;
}

// static
bool BasicIndexingQueue::shouldIndexContents(const QString& dir)
{
    return FileIndexerConfig::self()->shouldFolderBeIndexed(dir);
}

void BasicIndexingQueue::index(const FileMapping& file)
{
    kDebug() << file.id() << file.url();
    Q_EMIT beginIndexingFile(file);

    BasicIndexingJob job(&m_db->sqlDatabase(), file, m_currentMimeType);
    if (job.index()) {
        Q_EMIT newDocument(job.id(), job.document());
    }

    QTimer::singleShot(0, this, SLOT(slotIndexingFinished()));
}

void BasicIndexingQueue::slotIndexingFinished()
{
    FileMapping file = m_currentFile;
    m_currentFile.clear();
    m_currentMimeType.clear();
    m_currentFlags = NoUpdateFlags;

    Q_EMIT endIndexingFile(file);

    // Continue the queue
    finishIteration();
}
