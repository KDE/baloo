/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2014 Vishesh Handa <handa.vish@gmail.com>

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

#include <QDBusConnection>

using namespace Baloo;

FileIndexer::FileIndexer(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_startupUpdateDone(false)
{
    Q_ASSERT(config);

    // setup the actual index scheduler
    // ==============================================================
    m_indexScheduler = new IndexScheduler(db, config, this);

    // setup status connections
    connect(m_indexScheduler, SIGNAL(statusStringChanged()),
            this, SIGNAL(statusChanged()));
    connect(m_indexScheduler, SIGNAL(indexingStarted()),
            this, SIGNAL(indexingStarted()));
    connect(m_indexScheduler, SIGNAL(indexingStopped()),
            this, SIGNAL(indexingStopped()));
    connect(m_indexScheduler, SIGNAL(fileIndexingDone()),
            this, SIGNAL(fileIndexingDone()));
    connect(m_indexScheduler, SIGNAL(basicIndexingDone()),
            this, SLOT(slotBasicIndexingDone()));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QLatin1String("/indexer"), this,
                       QDBusConnection::ExportScriptableSignals |
                       QDBusConnection::ExportScriptableSlots |
                       QDBusConnection::ExportAdaptors);
}

FileIndexer::~FileIndexer()
{
}

void FileIndexer::update()
{
    if (m_startupUpdateDone)
        return;

    // start initial indexing honoring the hidden config option to disable it
    if (m_config->isInitialRun() || !m_config->initialUpdateDisabled()) {
        m_indexScheduler->updateAll();
    }
    m_startupUpdateDone = true;
}

void FileIndexer::slotBasicIndexingDone()
{
    m_config->setInitialRun(false);
}

QString FileIndexer::statusMessage() const
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

void FileIndexer::suspend() const
{
    m_indexScheduler->suspend();
}


void FileIndexer::resume() const
{
    m_indexScheduler->resume();
}

void FileIndexer::updateAllFolders(bool forced)
{
    m_indexScheduler->updateAll(forced);
}


void FileIndexer::indexFile(const QString& path)
{
    QFileInfo info(path);
    if (info.isDir()) {
        indexFolder(path, false /*non-recursive*/, false /*not-forced*/);
    }
    else {
        m_indexScheduler->indexFile(path);
    }
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

void FileIndexer::removeFileData(int id)
{
    m_indexScheduler->removeFileData(id);
}
