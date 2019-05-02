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

#include "baloodebug.h"

#include <QStandardPaths>
#include <QDataStream>

using namespace Baloo;

ExtractorProcess::ExtractorProcess(QObject* parent)
    : QObject(parent)
    , m_extractorPath(QStandardPaths::findExecutable(QStringLiteral("baloo_file_extractor")))
    , m_extractorProcess(this)
{
    connect(&m_extractorProcess, &QProcess::readyRead, this, &ExtractorProcess::slotIndexingFile);
    connect(&m_extractorProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                if (exitStatus == QProcess::CrashExit) {
                    Q_EMIT failed();
                }
            });

    m_extractorProcess.setProgram(m_extractorPath);
    m_extractorProcess.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    m_extractorProcess.start();
}

ExtractorProcess::~ExtractorProcess()
{
    m_extractorProcess.close();
}

void ExtractorProcess::start()
{
    m_extractorProcess.start(QIODevice::Unbuffered | QIODevice::ReadWrite);
    m_extractorProcess.waitForStarted();
    m_extractorProcess.setReadChannel(QProcess::StandardOutput);
}

void ExtractorProcess::index(const QVector<quint64>& fileIds)
{
    Q_ASSERT(!fileIds.isEmpty());

    QDataStream batch(&m_extractorProcess);
    batch << fileIds;
}

void ExtractorProcess::slotIndexingFile()
{
    while (m_extractorProcess.canReadLine()) {
        QString line = m_extractorProcess.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        char command = line[0].toLatin1();
        QString arg = line.mid(2);

        switch (command) {
        case 'S':
            Q_EMIT startedIndexingFile(arg);
            break;

        case 'F':
            Q_EMIT finishedIndexingFile(arg);
            break;

        case 'B':
            Q_EMIT done();
            break;

        default:
            qCritical(BALOO) << "Got unknown result from extractor" << command << arg;
        }
    }
}
