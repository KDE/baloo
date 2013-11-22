/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BALOO_INDEXING_JOB_H_
#define _BALOO_INDEXING_JOB_H_

#include <KJob>
#include <KUrl>
#include <QProcess>

#include "filemapping.h"

class QTimer;
class Database;

namespace Baloo
{

class FileIndexingJob : public KJob
{
    Q_OBJECT

public:
    FileIndexingJob(const QList<FileMapping>& files, QObject* parent = 0);

    virtual void start();

    /**
     * Error codes: IndexerFailed is emitted when the indexer returns 1
     *              IndexerCrashed is emitted when the indexer crashed
     */
    enum {
        IndexerFailed = UserDefinedError,
        IndexerCrashed
    };

    QList<FileMapping> files() const { return m_files; }

Q_SIGNALS:
    void deleteDocument(unsigned docid);

private Q_SLOTS:
    void slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessTimerTimeout();

private:
    QList<FileMapping> m_files;
    QProcess* m_process;
    QTimer* m_processTimer;
};
}

#endif
