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

#ifndef BALOO_MIGRATOR_H
#define BALOO_MIGRATOR_H

#include <QString>

namespace Baloo {

class FileIndexerConfig;

class Migrator
{
public:
    Migrator(const QString& dbPath, FileIndexerConfig* config);

    bool migrationRequired();
    void migrate();

private:
    QString m_dbPath;
    FileIndexerConfig* m_config;
};
}

#endif // BALOO_MIGRATOR_H
