/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
   Copyrigth (C) 2013 Vishesh Handa <me@vhanda.in>

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

#ifndef BALOO_INDEXING_JOB_H_
#define BALOO_INDEXING_JOB_H_

#include <KJob>
#include <QProcess>
#include <QVector>
#include <QStack>

class QTimer;

namespace Baloo
{

class FileIndexingJob : public KJob
{
    Q_OBJECT

public:
    FileIndexingJob(const QVector<quint64>& files, QObject* parent = 0);

    /**
     * Set a custom path which should be sent to the baloo_file_extractor
     * to use for the database. This is useful when debugging.
     */
    void setCustomDbPath(const QString& path);

    /**
     * Set the maximum number of msecs that each file should take in order
     * to get indexed. If a file takes longer, then it will be marked
     * as failing and the indexingFailed signal will be called
     *
     * By deafult this is 5 minutes
     */
    void setTimeoutInterval(int msec);

    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:
    /**
     * This signal is emitted when the indexing fails on a particular document
     */
    void indexingFailed(quint64 document);

protected:
    bool doSuspend() Q_DECL_OVERRIDE;
    bool doResume() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessTimerTimeout();

private:
    void start(const QVector<quint64>& files);

    /// holds the files which still need to be indexed
    QStack< QVector<quint64> > m_files;

    /// holds the files which have been sent to the process
    QVector<quint64> m_args;

    QProcess* m_process;
    QTimer* m_processTimer;
    int m_processTimeout;

    QString m_customDbPath;

    bool m_suspended;
};
}

#endif
