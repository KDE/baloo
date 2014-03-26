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

#ifndef _BALOO_FILE_DATABASE_H
#define _BALOO_FILE_DATABASE_H

#include <QObject>
#include <QSqlDatabase>

#include "xapiandatabase.h"

class Database : public QObject
{
public:
    explicit Database(QObject* parent = 0);
    ~Database();

    QString path();
    void setPath(const QString& path);

    bool init();
    bool isInitialized();

    QSqlDatabase& sqlDatabase();
    Baloo::XapianDatabase* xapianDatabase();

private:
    QString m_path;
    QString m_connectionName;
    bool m_initialized;

    Baloo::XapianDatabase* m_xapianDb;
    QSqlDatabase* m_sqlDb;
};

#endif // _BALOO_FILE_DATABASE_H
