/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

#ifndef BALOO_UNINDEXEDFILEINDEXER
#define BALOO_UNINDEXEDFILEINDEXER

#include <QRunnable>
#include <QObject>

namespace Baloo {

class Database;
class FileIndexerConfig;

class UnindexedFileIndexer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    UnindexedFileIndexer(Database* db, const FileIndexerConfig* config);

    void run() override;

Q_SIGNALS:
    void done();

private:
    Database* m_db;
    const FileIndexerConfig* m_config;
};
}

#endif //BALOO_UNINDEXEDFILEINDEXER
