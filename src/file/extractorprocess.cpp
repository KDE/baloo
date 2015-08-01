/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

#include "extractorprocess.h"

#include <QStandardPaths>
#include <QDebug>

using namespace Baloo;

ExtractorProcess::ExtractorProcess(QObject* parent)
    : QObject(parent)
    , m_extractorPath(QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor")))
    , m_extractorProcess(0)
    , m_extractorIdle(true)
{
    m_extractorProcess = new QProcess(this);
    connect(m_extractorProcess, &QProcess::readyRead, this, &ExtractorProcess::slotFileIndexed);
    m_extractorProcess->start(m_extractorPath, QStringList(), QIODevice::Unbuffered | QIODevice::ReadWrite);
    m_extractorProcess->waitForStarted();
    m_extractorProcess->setReadChannel(QProcess::StandardOutput);
}

ExtractorProcess::~ExtractorProcess()
{
    m_extractorProcess->disconnect(this);
    m_extractorProcess->deleteLater();
    m_extractorProcess = 0;
}

void ExtractorProcess::index(const QVector<quint64>& fileIds)
{
    Q_ASSERT(m_extractorProcess->state() == QProcess::Running);
    Q_ASSERT(m_extractorIdle);
    Q_ASSERT(!fileIds.isEmpty());

    QByteArray batchData;

    quint32 size = fileIds.size();
    batchData.append(reinterpret_cast<char*>(&size), sizeof(quint32));
    for (quint64 id : fileIds) {
        batchData.append(reinterpret_cast<char*>(&id), sizeof(quint64));
    }

    m_extractorIdle = false;
    m_extractorProcess->write(batchData.data(), batchData.size());
}

void ExtractorProcess::slotFileIndexed()
{
    QByteArray output = m_extractorProcess->readAll();
    Q_UNUSED(output);
    m_extractorIdle = true;

    Q_EMIT done();
}
