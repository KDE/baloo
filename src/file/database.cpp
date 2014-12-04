/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "database.h"

#include <QStringList>
#include <QDir>

#include <QDebug>

Database::Database(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_xapianDb(0)

{
}

Database::~Database()
{
    delete m_xapianDb;
}

bool Database::init()
{
    if (m_initialized)
        return true;

    // Create the Xapian DB
    m_xapianDb = new Baloo::XapianDatabase(m_path);

    m_initialized = true;
    return true;
}

QString Database::path()
{
    return m_path;
}

void Database::setPath(const QString& path)
{
    m_path = path;
    if (!m_path.endsWith(QLatin1Char('/')))
        m_path.append(QLatin1Char('/'));

    QDir().mkpath(m_path);

    QFileInfo dirInfo(path);
    if (!dirInfo.permission(QFile::WriteOwner)) {
        qCritical() << path << "does not have write permissions. Aborting";
        exit(1);
    }
}

bool Database::isInitialized()
{
    return m_initialized;
}

Baloo::XapianDatabase* Database::xapianDatabase()
{
    return m_xapianDb;
}

