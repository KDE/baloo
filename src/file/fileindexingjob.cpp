/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2012-2014 Vishesh Handa <me@vhanda.in>

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

#include "fileindexingjob.h"
#include "util.h"
#include "fileindexerconfig.h"
#include "database.h"

#include <QDebug>
#include <QStandardPaths>

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

using namespace Baloo;

FileIndexingJob::FileIndexingJob(const QVector<uint>& files, QObject* parent)
    : KJob(parent)
    , m_process(0)
    , m_suspended(false)
{
    Q_ASSERT(!files.isEmpty());
    m_files.push(files);

    setCapabilities(Suspendable);

    // setup the timer used to kill the indexer process if it seems to get stuck
    m_processTimer = new QTimer(this);
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, SIGNAL(timeout()),
            this, SLOT(slotProcessTimerTimeout()));

    m_processTimeout = 5 * 60 * 1000;
}

void FileIndexingJob::start()
{
    m_args = m_files.pop();
    start(m_args);
}

void FileIndexingJob::start(const QVector<uint>& files)
{
    // setup the external process which does the actual indexing
    static const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));

    Q_ASSERT(m_process == 0);
    m_process = new QProcess(this);

    QStringList args;
    Q_FOREACH (const uint& file, files)
        args << QString::number(file);

    if (!m_customDbPath.isEmpty()) {
        args << QLatin1String("--db") << m_customDbPath;
    }
    qDebug() << args;

    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotIndexedFile(int,QProcess::ExitStatus)));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->start(exe, args);

    m_processTimer->start(m_processTimeout);
}

void FileIndexingJob::slotIndexedFile(int, QProcess::ExitStatus exitStatus)
{
    // stop the timer since there is no need to kill the process anymore
    m_processTimer->stop();
    m_process->disconnect(this);
    m_process->deleteLater();
    m_process = 0;

    if (exitStatus == QProcess::NormalExit) {
        if (m_files.isEmpty()) {
            emitResult();
        } else {
            m_args = m_files.pop();
            if (!m_suspended) {
                start(m_args);
            }
        }
        return;
    }

    // Failed to index. We must figure out which was the offending file
    qDebug() << "Indexing failed. Trying to determine offending file";

    // Here it is!
    if (m_args.size() == 1) {
        uint doc = m_args.first();
        qWarning() << "Indexer crashed while indexing" << doc;
        qWarning() << "Blacklisting this file";
        Q_EMIT indexingFailed(doc);

        if (m_files.isEmpty()) {
            emitResult();
        } else {
            m_args = m_files.pop();
            if (!m_suspended) {
                start(m_args);
            }
        }
        return;
    }

    // We split the args into half and push the rest back into m_files
    // to call later
    int s = m_args.size() / 2;
    m_files.push(m_args.mid(s));
    m_args.resize(s);

    if (!m_suspended) {
        start(m_args);
    }
}

void FileIndexingJob::slotProcessTimerTimeout()
{
    // Emulate a crash so that we narrow down the file which is taking too long
    qDebug() << "Process took too long killing";
    slotIndexedFile(1, QProcess::CrashExit);
}

void FileIndexingJob::setCustomDbPath(const QString& path)
{
    m_customDbPath = path;
}

void FileIndexingJob::setTimeoutInterval(int msec)
{
    m_processTimeout = msec;
}

bool FileIndexingJob::doSuspend()
{
    if (m_suspended)
        return false;

    m_suspended = true;
    return true;
}

bool FileIndexingJob::doResume()
{
    if (!m_suspended)
        return false;

    m_suspended = false;
    if (!m_process)
        start(m_args);
    return true;
}


#include "fileindexingjob.moc"
