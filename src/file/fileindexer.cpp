/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2011 Vishesh Handa <handa.vish@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "fileindexer.h"
#include "indexscheduler.h"
#include "fileindexerconfig.h"
#include "util.h"

#include <KDebug>
#include <KDirNotify>

#include <QtCore/QTimer>

using namespace Baloo;

FileIndexer::FileIndexer(Database* db, QObject* parent)
    : QObject(parent)
{
    // Create the configuration instance singleton (for thread-safety)
    // ==============================================================
    (void)new FileIndexerConfig(this);

    // setup the actual index scheduler
    // ==============================================================
    m_indexScheduler = new IndexScheduler(db, this);

    // setup status connections
    connect(m_indexScheduler, SIGNAL(statusStringChanged()),
            this, SIGNAL(statusStringChanged()));

    // start initial indexing honoring the hidden config option to disable it
    if (FileIndexerConfig::self()->isInitialRun() || !FileIndexerConfig::self()->initialUpdateDisabled()) {
        m_indexScheduler->updateAll();
    }

    // Connect some signals used in the DBus interface
    connect(this, SIGNAL(statusStringChanged()),
            this, SIGNAL(statusChanged()));
    connect(m_indexScheduler, SIGNAL(indexingStarted()),
            this, SIGNAL(indexingStarted()));
    connect(m_indexScheduler, SIGNAL(indexingStopped()),
            this, SIGNAL(indexingStopped()));
    connect(m_indexScheduler, SIGNAL(fileIndexingDone()),
            this, SIGNAL(fileIndexingDone()));
    connect(m_indexScheduler, SIGNAL(basicIndexingDone()),
            this, SLOT(slotIndexingDone()));

    connect(m_indexScheduler, SIGNAL(statusStringChanged()),
            this, SLOT(emitStatusMessage()));

}


FileIndexer::~FileIndexer()
{
}

void FileIndexer::slotIndexingDone()
{
    FileIndexerConfig::self()->setInitialRun(false);
}

void FileIndexer::emitStatusMessage()
{
    QString message = m_indexScheduler->userStatusString();

    Q_EMIT status((int)m_indexScheduler->currentStatus(), message);
}

QString FileIndexer::statusMessage() const
{
    return m_indexScheduler->userStatusString();
}

int FileIndexer::currentStatus() const
{
    return (int)m_indexScheduler->currentStatus();
}

QString FileIndexer::userStatusString() const
{
    return m_indexScheduler->userStatusString();
}

void FileIndexer::setSuspended(bool suspend)
{
    if (suspend) {
        m_indexScheduler->suspend();
    } else {
        m_indexScheduler->resume();
    }
}


bool FileIndexer::isSuspended() const
{
    return m_indexScheduler->isSuspended();
}


bool FileIndexer::isIndexing() const
{
    return m_indexScheduler->isIndexing();
}

bool FileIndexer::isCleaning() const
{
    return m_indexScheduler->isCleaning();
}


void FileIndexer::suspend() const
{
    m_indexScheduler->suspend();
}


void FileIndexer::resume() const
{
    m_indexScheduler->resume();
}


QString FileIndexer::currentFile() const
{
    return m_indexScheduler->currentUrl().toLocalFile();
}


QString FileIndexer::currentFolder() const
{
    return KUrl(m_indexScheduler->currentUrl()).directory();
}


void FileIndexer::updateFolder(const QString& path, bool recursive, bool forced)
{
    kDebug() << "Called with path: " << path;
    QFileInfo info(path);
    if (info.exists()) {
        QString dirPath;
        if (info.isDir())
            dirPath = info.absoluteFilePath();
        else
            dirPath = info.absolutePath();

        if (FileIndexerConfig::self()->shouldFolderBeIndexed(dirPath)) {
            indexFolder(path, recursive, forced);
        }
    }
}

/*
int FileIndexer::indexedFiles() const
{
    QString query = QString::fromLatin1("select count(distinct ?r) where { ?r kext:indexingLevel ?t. "
                                        " FILTER(?t >= %1) . }")
                    .arg(Soprano::Node::literalToN3(Soprano::LiteralValue(2)));

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    if (it.next())
        return it[0].literal().toInt();

    return 0;
}

int FileIndexer::totalFiles() const
{
    QString query = QString::fromLatin1("select count(distinct ?r) where { ?r kext:indexingLevel ?t. }");

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    if (it.next())
        return it[0].literal().toInt();

    return 0;
}
*/


void FileIndexer::updateAllFolders(bool forced)
{
    m_indexScheduler->updateAll(forced);
}


void FileIndexer::indexFile(const QString& path)
{
    m_indexScheduler->analyzeFile(path);
}


void FileIndexer::indexFolder(const QString& path, bool recursive, bool forced)
{
    QFileInfo info(path);
    if (info.exists()) {
        QString dirPath;
        if (info.isDir())
            dirPath = info.absoluteFilePath();
        else
            dirPath = info.absolutePath();

        kDebug() << "Updating : " << dirPath;

        UpdateDirFlags flags;
        if (recursive)
            flags |= UpdateRecursive;
        if (forced)
            flags |= ForceUpdate;

        m_indexScheduler->updateDir(dirPath, flags);
    }
}
