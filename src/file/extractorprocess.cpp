/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractorprocess.h"

#include "baloodebug.h"
#include "config.h"


using namespace Baloo;

ExtractorProcess::ExtractorProcess(QObject* parent)
    : QObject(parent)
    , m_extractorPath(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR "/baloo_file_extractor"))
    , m_extractorProcess(this)
    , m_controller(&m_extractorProcess, &m_extractorProcess)
{
    using ControllerPipe = Baloo::Private::ControllerPipe;

    connect(&m_extractorProcess, &QProcess::readyRead, &m_controller, &ControllerPipe::processStatusData);
    connect(&m_controller, &ControllerPipe::urlStarted, this, &ExtractorProcess::startedIndexingFile);
    connect(&m_controller, &ControllerPipe::urlFinished, this, [this](const QString& url) {
        Q_EMIT finishedIndexingFile(url, true);
    });
    connect(&m_controller, &ControllerPipe::urlFailed, this, [this](const QString& url) {
        Q_EMIT finishedIndexingFile(url, false);
    });
    connect(&m_controller, &ControllerPipe::batchFinished, this, [this]() {
        Q_EMIT done();
    });

    connect(&m_extractorProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
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
    m_extractorProcess.closeWriteChannel();
    m_extractorProcess.waitForFinished();
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
    m_controller.processIds(fileIds);
}

