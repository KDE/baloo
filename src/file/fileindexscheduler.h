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

#ifndef BALOO_FILEINDEXSCHEDULER_H
#define BALOO_FILEINDEXSCHEDULER_H

#include <QObject>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>

namespace Baloo {

class Database;
class FileIndexerConfig;

class FileIndexScheduler : public QObject
{
    Q_OBJECT
public:
    FileIndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent = 0);

public Q_SLOTS:
    void indexNewFile(const QString& file) {
        m_newFiles << file;
        QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
    }

    void indexModifiedFile(const QString& file) {
        m_modifiedFiles << file;
        QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
    }

    void indexXAttrFile(const QString& file) {
        m_xattrFiles << file;
        QTimer::singleShot(0, this, SLOT(scheduleIndexing()));
    }

    void scheduleIndexing();
private:
    Database* m_db;
    FileIndexerConfig* m_config;

    QStringList m_newFiles;
    QStringList m_modifiedFiles;
    QStringList m_xattrFiles;

    QThreadPool m_threadPool;
};

}

#endif // BALOO_FILEINDEXSCHEDULER_H
