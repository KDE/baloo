/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "fileindexscheduler.h"

#include "firstrunindexer.h"
#include "newfileindexer.h"
#include "modifiedfileindexer.h"
#include "xattrindexer.h"
#include "filecontentindexer.h"

#include "fileindexerconfig.h"

#include <QTimer>
#include <QDebug>

using namespace Baloo;

FileIndexScheduler::FileIndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    m_threadPool.setMaxThreadCount(1);
}

void FileIndexScheduler::scheduleIndexing()
{
    if (m_threadPool.activeThreadCount()) {
        return;
    }
    qDebug() << "SCHEDULE";

    if (m_config->isInitialRun()) {
        qDebug() << m_config->includeFolders();
        auto runnable = new FirstRunIndexer(m_db, m_config, m_config->includeFolders());
        connect(runnable, &FirstRunIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        return;
    }

    if (!m_newFiles.isEmpty()) {
        qDebug() << "NEW" << m_newFiles;
        auto runnable = new NewFileIndexer(m_db, m_config, m_newFiles);
        connect(runnable, &NewFileIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_newFiles.clear();
        return;
    }

    if (!m_modifiedFiles.isEmpty()) {
        qDebug() << "MOD" << m_modifiedFiles;
        auto runnable = new ModifiedFileIndexer(m_db, m_config, m_modifiedFiles);
        connect(runnable, &ModifiedFileIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_modifiedFiles.clear();
        return;
    }

    if (!m_xattrFiles.isEmpty()) {
        qDebug() << "XATTR" << m_xattrFiles;
        auto runnable = new XAttrIndexer(m_db, m_config, m_xattrFiles);
        connect(runnable, &XAttrIndexer::done, this, &FileIndexScheduler::scheduleIndexing);

        m_threadPool.start(runnable);
        m_xattrFiles.clear();
        return;
    }

    qDebug() << "IDLE";
}
