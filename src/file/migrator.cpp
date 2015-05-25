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

#include "migrator.h"
#include "fileindexerconfig.h"

#include <QFile>
#include <QDir>

using namespace Baloo;

Migrator::Migrator(const QString& dbPath, FileIndexerConfig* config)
    : m_dbPath(dbPath)
    , m_config(config)
{
    Q_ASSERT(!dbPath.isEmpty());
    Q_ASSERT(!dbPath.endsWith('/'));
    Q_ASSERT(config);
}

/*
 * Changing this version number indicates that the old index should be deleted
 * and the indexing should be started from scratch.
 */
static int s_dbVersion = 1;

bool Migrator::migrationRequired()
{
    return m_config->databaseVersion() != s_dbVersion;
}

void Migrator::migrate()
{
    Q_ASSERT(migrationRequired());

    int dbVersion = m_config->databaseVersion();
    if (dbVersion == 0 && QFile::exists(m_dbPath + "/file")) {
        QDir dir(m_dbPath + "/file");
        dir.removeRecursively();
    }
    else if (dbVersion == 1 && QFile::exists(m_dbPath + "/index")) {
        Q_ASSERT(0);
    }

    m_config->setDatabaseVersion(s_dbVersion);
    m_config->setInitialRun(true);
}
