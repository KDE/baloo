/*
   This file is part of the KDE Baloo project.
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

#include "enginesearchstore.h"
#include "query.h"

#include "database.h"
#include "enginequery.h"

#include <QStandardPaths>

using namespace Baloo;

EngineSearchStore::EngineSearchStore()
    : m_db(0)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
    m_db = new Database(path);
    m_db->open();
    m_db->transaction();
}

EngineSearchStore::~EngineSearchStore()
{
    delete m_db;
}

QStringList EngineSearchStore::types()
{
    return QStringList() << QLatin1String("Audio") << QLatin1String("Video") << QLatin1String("Document")
                         << QLatin1String("Image") << QLatin1String("Archive") << QLatin1String("Folder");
}

QVector<uint> EngineSearchStore::exec(const Query& query)
{
    if (!query.searchString().isEmpty()) {
        EngineQuery eq(query.searchString().toUtf8());
        return m_db->exec(eq);
    }

    return QVector<uint>();
}

QString EngineSearchStore::filePath(uint id)
{
    Q_ASSERT(id > 0);
    return m_db->documentUrl(id);
}
