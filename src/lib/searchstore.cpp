/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "searchstore.h"
#include "query.h"

#include "database.h"
#include "enginequery.h"

#include <QStandardPaths>

using namespace Baloo;

SearchStore::SearchStore()
    : m_db(0)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
    m_db = new Database(path);
    m_db->open();
    m_db->transaction();
}

SearchStore::~SearchStore()
{
    delete m_db;
}

QStringList SearchStore::types()
{
    return QStringList() << QLatin1String("Audio") << QLatin1String("Video") << QLatin1String("Document")
                         << QLatin1String("Image") << QLatin1String("Archive") << QLatin1String("Folder");
}

QVector<uint> SearchStore::exec(const Query& query)
{
    if (!query.searchString().isEmpty()) {
        EngineQuery eq(query.searchString().toUtf8());
        return m_db->exec(eq);
    }

    return QVector<uint>();
}

QString SearchStore::filePath(uint id)
{
    Q_ASSERT(id > 0);
    return m_db->documentUrl(id);
}
