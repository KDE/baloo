/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractorprocess.h"

#include "baloodebug.h"

#include <QStandardPaths>
#include <QDataStream>
#include <QTextStream>

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
                if (exitCode == 1) {
                    // DB open error
                    Q_EMIT failed();
                }
                if (exitCode == 2) {
                    // DB transaction commit error
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
    QString line;
    QTextStream stream(&m_extractorProcess);
    while (m_extractorProcess.canReadLine()) {
        char command, space;
        stream >> command >> space;

        if (!stream.readLineInto(&line)) {
            break;
        }

        const QString arg = line.trimmed();
        if (arg.isEmpty()) {
            continue;
        }

        switch (command) {
        case 'S':
            Q_EMIT startedIndexingFile(arg);
            break;

        case 'f':
            Q_EMIT finishedIndexingFile(arg, false);
            break;

        case 'F':
            Q_EMIT finishedIndexingFile(arg, true);
            break;

        case 'B':
            Q_EMIT done();
            break;

        default:
            qCritical(BALOO) << "Got unknown result from extractor" << command << arg;
        }
    }
}
