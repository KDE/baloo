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

#include "fileextractor.h"

#include <QTime>
#include <QTimer>
#include <QUrl>

#include <QStandardPaths>
#include <QMimeDatabase>

using namespace Baloo;

FileExtractor::FileExtractor(uint id, const QString& url)
    : KJob()
    , m_id(id)
    , m_url(url)
    , m_process(0)
    , m_elapsed(0)
{
}

void FileExtractor::setCustomPath(const QString& path)
{
    m_customPath = path;
}

void FileExtractor::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileExtractor::doStart()
{
    // Get the mimetype - used for stats later
    m_mimeType = QMimeDatabase().mimeTypeForFile(m_url, QMimeDatabase::MatchExtension).name();

    // setup the external process which does the actual indexing
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << QString::number(m_id);
    args << QLatin1String("--db") << m_customPath;

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotIndexedFile(int,QProcess::ExitStatus)));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    m_timer.start();
    m_process->start(exe, args);
}

void FileExtractor::slotIndexedFile(int, QProcess::ExitStatus)
{
    m_elapsed = m_timer.elapsed();

    emitResult();
    return;
}

int FileExtractor::elapsed() const
{
    return m_elapsed;
}

QString FileExtractor::mimeType() const
{
    return m_mimeType;
}

