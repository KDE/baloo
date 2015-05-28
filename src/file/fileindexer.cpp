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
#include "baloodebug.h"

#include <QDBusConnection>
#include <QCoreApplication>

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
    connect(m_indexScheduler, &IndexScheduler::statusStringChanged, this, &FileIndexer::statusChanged);
    connect(m_indexScheduler, &IndexScheduler::indexingStarted, this, &FileIndexer::indexingStarted);
    connect(m_indexScheduler, &IndexScheduler::indexingStopped, this, &FileIndexer::indexingStopped);
    connect(m_indexScheduler, &IndexScheduler::fileIndexingDone, this, &FileIndexer::fileIndexingDone);
    connect(m_indexScheduler, &IndexScheduler::basicIndexingDone, this, &FileIndexer::slotBasicIndexingDone);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QLatin1String("/indexer"), this,
                       QDBusConnection::ExportScriptableSignals |
                       QDBusConnection::ExportScriptableSlots |
                       QDBusConnection::ExportAdaptors);

    m_initalRun = m_config->isInitialRun();
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
    if (m_initalRun) {
        m_config->setInitialRun(false);
        m_initalRun = false;
    }
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
        indexFolder(path, false /*not-forced*/);
    }
    else {
        m_indexScheduler->indexFile(path);
    }
}

void FileIndexer::indexXAttr(const QString& path)
{
    m_indexScheduler->indexXattr(path);
}


void FileIndexer::indexFolder(const QString& path, bool forced)
{
    QFileInfo info(path);
    if (info.exists()) {
        QString dirPath;
        if (info.isDir())
            dirPath = info.absoluteFilePath();
        else
            dirPath = info.absolutePath();

        qCDebug(BALOO) << "Updating : " << dirPath;

        UpdateDirFlags flags;
        if (forced)
            flags |= ForceUpdate;

        m_indexScheduler->updateDir(dirPath, flags);
    }
}

void FileIndexer::quit() const
{
    QCoreApplication::instance()->quit();
}

void FileIndexer::updateConfig()
{
    m_indexScheduler->updateConfig();
}
