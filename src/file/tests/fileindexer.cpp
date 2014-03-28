/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "fileindexer.h"

#include <QTime>
#include <QTimer>

#include <KStandardDirs>
#include <KMimeType>

using namespace Baloo;

FileIndexer::FileIndexer(uint id, const QString& url)
    : KJob()
    , m_id(id)
    , m_url(url)
    , m_process(0)
{
}

void FileIndexer::setCustomPath(const QString& path)
{
    m_customPath = path;
}

void FileIndexer::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileIndexer::doStart()
{
    // Get the mimetype - used for stats later
    m_mimeType = KMimeType::findByUrl(QUrl::fromLocalFile(m_url))->name();

    // setup the external process which does the actual indexing
    const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << QString::number(m_id);
    args << "--db" << m_customPath;

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotIndexedFile(int,QProcess::ExitStatus)));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    m_timer.start();
    m_process->start(exe, args);
}

void FileIndexer::slotIndexedFile(int returnCode, QProcess::ExitStatus status)
{
    m_elapsed = m_timer.elapsed();

    emitResult();
    return;
}

int FileIndexer::elapsed() const
{
    return m_elapsed;
}

QString FileIndexer::mimeType() const
{
    return m_mimeType;
}

