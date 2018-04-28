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

#ifndef BALOO_MAINHUB_H
#define BALOO_MAINHUB_H

#include <QObject>

#include "filewatch.h"
#include "fileindexscheduler.h"

#include <QList>

namespace Baloo {

class Database;
class FileIndexerConfig;

class MainHub : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.main")
public:
    MainHub(Database* db, FileIndexerConfig* config);

public Q_SLOTS:
    Q_SCRIPTABLE void quit() const;
    Q_SCRIPTABLE void updateConfig();
    Q_SCRIPTABLE void registerBalooWatcher(const QString &service);

private:
    Database* m_db;
    FileIndexerConfig* m_config;

    FileWatch m_fileWatcher;
    FileIndexScheduler m_fileIndexScheduler;

};
}

#endif // BALOO_MAINHUB_H
