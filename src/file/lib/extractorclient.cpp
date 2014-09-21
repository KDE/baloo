/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
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

#include "extractorclient.h"

#include <QDebug>
#include <QProcess>
#include <QSocketNotifier>
#include <QStandardPaths>
#include <QTextStream>

namespace Baloo
{


class ExtractorClient::Private
{
public:
    enum CommandState {
        NoCommand = 0,
        Indexed = 1,
        Saved = 2,
        BinaryData = 3
    };

    Private(ExtractorClient *parent)
        : q(parent),
          extractor(new QProcess(q)),
          writeStream(commandQueue, QIODevice::WriteOnly),
          commandState(NoCommand)
    {
    }

    void extractorStarted();
    void readResponse();
    void extractorDead();

    ExtractorClient *q;
    QProcess *extractor;
    QTextStream writeStream;
    QByteArray commandQueue;
    CommandState commandState;
    int binaryDataLength;
    QByteArray binaryDataBuffer;
};

void ExtractorClient::Private::extractorStarted()
{
    writeStream.setDevice(extractor);
    if (!commandQueue.isEmpty()) {
        writeStream << commandQueue;
        commandQueue.clear();
    }
}

void ExtractorClient::Private::extractorDead()
{
    extractor = 0;
    Q_EMIT q->extractorDied();
}

void ExtractorClient::Private::readResponse()
{
    if (!extractor) {
        return;
    }

    if (commandState == NoCommand) {
        char code;
        extractor->getChar(&code);

        switch (code) {
            case 'b': {
                commandState = BinaryData;
                binaryDataLength = 0;
                binaryDataBuffer.clear();
                break;
            }

            case 'i':
                commandState = Indexed;
                break;

            case 's':
                commandState = Saved;
                break;

            default:
                qDebug() << "Unknown response code: " << code;
                delete extractor;
                extractor = 0;
                Q_EMIT q->extractorDied();
                return;
                break;
        }
    }

    if (commandState == BinaryData) {
        if (binaryDataLength == 0) {
            qint64 read = extractor->read((char*)&binaryDataLength, sizeof(binaryDataLength));
            if (read < sizeof(binaryDataLength) || binaryDataLength < 1) {
                binaryDataLength = 0;
                commandState = NoCommand;
            }
        }

        if (binaryDataBuffer.size() < binaryDataLength) {
            const QByteArray newData = extractor->read(binaryDataLength - binaryDataBuffer.size());
            if (newData.isEmpty()) {
                commandState = NoCommand;
                binaryDataBuffer.clear();
            } else {
                binaryDataBuffer += newData;
            }
        }

        if (binaryDataBuffer.size() == binaryDataLength) {
            // get rid of the trailing newline
            if (extractor->read(1).size() == 1) {
                commandState = NoCommand;
                QVariantMap map;
                QDataStream s(&binaryDataBuffer, QIODevice::ReadOnly);
                s >> map;
                Q_EMIT q->binaryData(map);
            }
        }
    } else if (extractor->canReadLine()) {
        commandState = NoCommand;
        if (commandState == Indexed) {
            Q_EMIT q->fileIndexed(extractor->readLine());
        } else if (commandState == Saved) {
            Q_EMIT q->dataSaved(extractor->readLine());
        }
    }
}

ExtractorClient::ExtractorClient(QObject *parent)
    : QObject(parent),
      d(new Private(this))
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor_worker"));
    connect(d->extractor, SIGNAL(finish(int,QProcess::ExitStatus)), this, SLOT(extractorDead()));
    connect(d->extractor, SIGNAL(readyReadStandardOutput()), this, SLOT(readResponse()));
    connect(d->extractor, SIGNAL(started()), this, SLOT(extractorStarted()));
    d->extractor->setProcessChannelMode(QProcess::SeparateChannels);
    d->extractor->start(exe);
}

bool ExtractorClient::isValid() const
{
    return d->extractor;
}

void ExtractorClient::setBinaryOutput(bool binaryOutput)
{
    d->writeStream << 'b' << (binaryOutput ? '+' : '-');
}

void ExtractorClient::setFollowConfig(bool followConfig)
{
    d->writeStream << 'c' << (followConfig ? '+' : '-');
}

void ExtractorClient::setSaveToDatabase(bool saveToDatabase)
{
    d->writeStream << 'd' << (saveToDatabase ? '+' : '-');
}

void ExtractorClient::setDatabasePath(const QString &path)
{
    d->writeStream << 's' << path;
}

void ExtractorClient::enableDebuging(bool debugging)
{
    d->writeStream << 'z' << (debugging ? '+' : '-');
}

void ExtractorClient::indexFile(const QString &file)
{
    d->writeStream << 'i' << file;
}

} // namespace Baloo

#include "moc_extractorclient.cpp"

