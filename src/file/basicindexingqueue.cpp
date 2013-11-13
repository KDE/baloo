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

#include <KDebug>
#include <KMimeType>
#include <KStandardDirs>

#include <QtCore/QDateTime>
#include <QSqlQuery>

using namespace Baloo;

BasicIndexingQueue::BasicIndexingQueue(Database* db, QObject* parent)
    : IndexingQueue(parent)
    , m_db(db)
{

}

void BasicIndexingQueue::clear()
{
    m_currentUrl.clear();
    m_currentFlags = NoUpdateFlags;
    m_paths.clear();
    m_currentFileId = 0;
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

QString BasicIndexingQueue::currentUrl() const
{
    return m_currentUrl;
}

UpdateDirFlags BasicIndexingQueue::currentFlags() const
{
    return m_currentFlags;
}


bool BasicIndexingQueue::isEmpty()
{
    return m_paths.isEmpty();
}

void BasicIndexingQueue::enqueue(const QString& path)
{
    UpdateDirFlags flags;
    flags |= UpdateRecursive;

    enqueue(path, flags);
}

void BasicIndexingQueue::enqueue(const QString& path, UpdateDirFlags flags)
{
    kDebug() << path;
    m_paths.push(qMakePair(path, flags));
    callForNextIteration();

    Q_EMIT startedIndexing();
}

void BasicIndexingQueue::processNextIteration()
{
    bool processingFile = false;

    if (!m_paths.isEmpty()) {
        QPair< QString, UpdateDirFlags > pair = m_paths.pop();
        processingFile = process(pair.first, pair.second);
    }

    if (!processingFile)
        finishIteration();
}


bool BasicIndexingQueue::process(const QString& path, UpdateDirFlags flags)
{
    bool startedIndexing = false;

    QString mimetype = KMimeType::findByUrl(QUrl::fromLocalFile(path))->name();

    bool forced = flags & ForceUpdate;
    bool recursive = flags & UpdateRecursive;
    bool indexingRequired = shouldIndex(path, mimetype);

    QFileInfo info(path);
    if (info.isDir()) {
        if (forced || indexingRequired) {
            m_currentUrl = path;
            m_currentFlags = flags;
            m_currentMimeType = mimetype;

            startedIndexing = true;
            index(path);
        }

        // We don't want to follow system links
        if (recursive && !info.isSymLink() && shouldIndexContents(path)) {
            QDir::Filters dirFilter = QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs;

            QDirIterator it(path, dirFilter);
            while (it.hasNext()) {
                m_paths.push(qMakePair(it.next(), flags));
            }
        }
    } else if (info.isFile() && (forced || indexingRequired)) {
        m_currentUrl = path;
        m_currentFlags = flags;
        m_currentMimeType = mimetype;

        startedIndexing = true;
        index(path);
    }

    return startedIndexing;
}

bool BasicIndexingQueue::shouldIndex(const QString& path, const QString& mimetype)
{
    bool shouldIndexFile = FileIndexerConfig::self()->shouldFileBeIndexed(path);
    if (!shouldIndexFile)
        return false;

    bool shouldIndexType = FileIndexerConfig::self()->shouldMimeTypeBeIndexed(mimetype);
    if (!shouldIndexType)
        return false;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
        return false;

    QSqlQuery query(m_db->sqlDatabase());
    query.setForwardOnly(true);
    query.prepare(QLatin1String("select id from files where url = ?"));
    query.addBindValue(path);
    query.exec();

    if (query.next()) {
        m_currentFileId = query.value(0).toInt();
    }
    else {
        m_currentFileId = 0;
        return true;
    }

    try {
        Xapian::Document doc = m_db->xapainDatabase()->get_document(m_currentFileId);
        Xapian::TermIterator it = doc.termlist_begin();
        it.skip_to("DT_M");
        if (it == doc.termlist_end())
            return false;

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

bool BasicIndexingQueue::shouldIndexContents(const QString& dir)
{
    return FileIndexerConfig::self()->shouldFolderBeIndexed(dir);
}

void BasicIndexingQueue::index(const QString& path)
{
    kDebug() << path;
    Q_EMIT beginIndexingFile(path);

    KJob* job = new Baloo::BasicIndexingJob(m_db, m_currentFileId, path, m_currentMimeType);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotIndexingFinished(KJob*)));

    job->start();
}

void BasicIndexingQueue::slotIndexingFinished(KJob* job)
{
    if (job->error()) {
        kDebug() << job->errorString();
    }

    QString url = m_currentUrl;
    m_currentUrl.clear();
    m_currentMimeType.clear();
    m_currentFlags = NoUpdateFlags;
    m_currentFileId = 0;

    Q_EMIT endIndexingFile(url);

    // Continue the queue
    finishIteration();
}
