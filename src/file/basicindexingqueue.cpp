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

    // This mimetype may not be completely accurate, but that's okay. This is
    // just the initial phase of indexing. The second phase can try to find
    // a more accurate mimetype.
    QString mimetype = m_mimeDb.mimeTypeForFile(file, QMimeDatabase::MatchExtension).name();

    bool forced = flags & ForceUpdate;
    bool indexingRequired = (flags & ExtendedAttributesOnly) || shouldIndex(file, mimetype);

    QFileInfo info(file);
    if (info.isDir()) {
        if (forced || indexingRequired) {
            startedIndexing = true;
            index(file, mimetype, flags);
        }

        // We don't want to follow system links
        if (!info.isSymLink() && shouldIndexContents(file)) {
            QDir::Filters dirFilter = QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs;

            QDirIterator it(file, dirFilter);
            while (it.hasNext()) {
                m_paths.push(qMakePair(it.next(), flags));
            }
        }
    } else if (info.isFile() && (forced || indexingRequired)) {
        startedIndexing = true;
        index(file, mimetype, flags);
    }

    return startedIndexing;
}

bool BasicIndexingQueue::shouldIndex(const QString& file, const QString& mimetype) const
{
    bool shouldBeIndexed = m_config->shouldBeIndexed(file);
    if (!shouldBeIndexed)
        return false;

    bool shouldIndexType = m_config->shouldMimeTypeBeIndexed(mimetype);
    if (!shouldIndexType)
        return false;

    QFileInfo fileInfo(file);
    if (!fileInfo.exists())
        return false;

    quint64 fileId = m_db->documentId(file.toUtf8());
    if (!fileId) {
        return true;
    }

    const QByteArray dtStr = m_db->documentSlot(fileId, 0);
    if (dtStr.isEmpty()) {
        return true;
    }

    // A folders mtime is updated when a new file is added / removed / renamed
    // we don't really need to reindex a folder when that happens
    // In fact, we never need to reindex a folder
    if (mimetype == QLatin1String("inode/directory"))
        return false;

    const uint time_t = dtStr.toUInt();
    if (time_t != fileInfo.lastModified().toTime_t()) {
        return true;
    }

    return false;
}

bool BasicIndexingQueue::shouldIndexContents(const QString& dir)
{
    return m_config->shouldFolderBeIndexed(dir);
}

void BasicIndexingQueue::index(const QString& file, const QString& mimetype,
                               UpdateDirFlags flags)
{
    //qDebug() << file.url();

    bool xattrOnly = (flags & Baloo::ExtendedAttributesOnly);
    if (!xattrOnly) {
        BasicIndexingJob job(file, mimetype, m_config->onlyBasicIndexing());
        if (job.index()) {
            Q_EMIT newDocument(0, job.document());
        }
    }
    else {
        // FIXME: Re-index only the xattr. How!!
        /*
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
        */
    }

    QTimer::singleShot(0, this, SLOT(finishIteration()));
}
