/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commandpipe.h"
#include <QVector>
#include "baloodebug.h"
#include <QIODevice>
namespace Baloo {
namespace Private {

enum BatchStatus : quint8 {
    Invalid = 'X',
    UrlStarted = 'S',
    UrlFinished = 'F',
    UrlUnchanged = 'U',
    UrlFailed = 'f',
    UrlIgnored = 's',
    UrlInvalid = 'i',
    MimetypeIgnored = 'm',
    MimetypeUnsupported = 'M',
    FileTooLarge = 'l',
    BatchFinished = 'B',
};

ControllerPipe::ControllerPipe(QIODevice* commandPipe, QIODevice* statusPipe)
    : m_commandStream(commandPipe)
    , m_statusStream(statusPipe)
{
}

void ControllerPipe::processIds(const QVector<quint64>& ids)
{
    m_commandStream << ids;
}

void ControllerPipe::processStatusData()
{
    using FileIndexStatus = Baloo::IndexResult::FileStatus;

    QString url;
    BatchStatus event{Invalid};

    while (true) {
        m_statusStream.startTransaction();
        m_statusStream >> event;

        if ((m_statusStream.status() != QDataStream::Ok) && m_statusStream.device()->atEnd()) {
	    m_statusStream.rollbackTransaction();
            break;
        }

        if (event == BatchFinished) {
            if (m_statusStream.commitTransaction()) {
                Q_EMIT batchFinished();
                continue;
            } else {
                break;
            }
        }

        m_statusStream >> url;
        if (!m_statusStream.commitTransaction()) {
            break;
        }

        switch (event) {
        case UrlStarted:
            Q_EMIT urlStarted(url);
            break;

        case UrlFinished:
            Q_EMIT urlProcessed(url, true, FileIndexStatus::Successful);
            break;
        case UrlUnchanged:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::Successful);
            break;
        case UrlIgnored:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::IgnoredFilename);
            break;
        case MimetypeIgnored:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::IgnoredMimetype);
            break;
        case MimetypeUnsupported:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::IgnoredMimetypeUnsupported);
            break;
        case FileTooLarge:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::IgnoredTooLarge);
            break;
        case UrlFailed:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::ErrorExtractionFailed);
            break;
        case UrlInvalid:
            Q_EMIT urlProcessed(url, false, FileIndexStatus::ErrorFileNotFound);
            break;

        default:
            qCCritical(BALOO) << "Got unknown result from extractor" << event << url;
        }
    }
}

WorkerPipe::WorkerPipe(QIODevice* commandPipe, QIODevice* statusPipe)
    : m_commandStream(commandPipe)
    , m_statusStream(statusPipe)
{
}

void WorkerPipe::processIdData()
{
    QVector<quint64> ids;

    while (true) {
        m_commandStream.startTransaction();
        m_commandStream >> ids;

        /* QIODevice::atEnd() has to be checked *after* reading from
         * the QDataStream, but *before* commitTransaction to get
         * the correct pipe status.
         * QDataStream::status() will be `ReadPastEnd` for both partial
         * reads as well as closed pipes
         */
        if ((m_commandStream.status() != QDataStream::Ok) && m_commandStream.device()->atEnd()) {
            m_commandStream.rollbackTransaction();
            Q_EMIT inputEnd();
            return;
        }

        if (!m_commandStream.commitTransaction()) {
            return;
        }

        Q_EMIT newDocumentIds(ids);
        if (m_commandStream.device()->atEnd()) {
            return;
        }
    }
}

void WorkerPipe::urlStarted(const QString& url)
{
    m_statusStream << UrlStarted << url;
}

void WorkerPipe::urlProcessed(const QString &url, bool updated, Baloo::IndexResult::FileStatus status)
{
    using FileIndexStatus = Baloo::IndexResult::FileStatus;

    switch (status) {
    case FileIndexStatus::Successful:
        if (updated) {
            m_statusStream << UrlFinished << url;
        } else {
            m_statusStream << UrlUnchanged << url;
        }
        break;
    case FileIndexStatus::IgnoredFilename:
        m_statusStream << UrlIgnored << url;
        break;
    case FileIndexStatus::IgnoredMimetype:
        m_statusStream << MimetypeIgnored << url;
        break;
    case FileIndexStatus::IgnoredMimetypeUnsupported:
        m_statusStream << MimetypeUnsupported << url;
        break;
    case FileIndexStatus::IgnoredTooLarge:
        m_statusStream << FileTooLarge << url;
        break;
    case FileIndexStatus::ErrorExtractionFailed:
        m_statusStream << UrlFailed << url;
        break;
    case FileIndexStatus::ErrorFileNotFound:
        m_statusStream << UrlInvalid << url;
        break;
    }
}

void WorkerPipe::batchFinished()
{
    m_statusStream << BatchFinished;
}

} // namespace Private
} // namespace Baloo

#include "moc_commandpipe.cpp"
