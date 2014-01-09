/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-2011 Sebastian Trueg <trueg@kde.org>

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

#include <qjson/serializer.h>

#include <KDebug>
#include <KStandardDirs>

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

using namespace Baloo;

FileIndexingJob::FileIndexingJob(const QVector<uint>& files, QObject* parent)
    : KJob(parent)
    , m_files(files)
{
    // setup the timer used to kill the indexer process if it seems to get stuck
    m_processTimer = new QTimer(this);
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, SIGNAL(timeout()),
            this, SLOT(slotProcessTimerTimeout()));
}

void FileIndexingJob::start()
{
    if (m_files.isEmpty()) {
        emitResult();
        return;
    }

    // setup the external process which does the actual indexing
    const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_extractor"));

    m_process = new QProcess(this);

    QStringList args;
    Q_FOREACH (const uint& file, m_files)
        args << QString::number(file);
    kDebug() << args;

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotIndexedFile(int, QProcess::ExitStatus)));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->start(exe, args);

    // start the timer which will kill the process if it does not terminate after 5 minutes
    m_processTimer->start(5 * 60 * 1000);
}

void FileIndexingJob::slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus)
{
    // stop the timer since there is no need to kill the process anymore
    m_processTimer->stop();

    if (exitStatus != QProcess::NormalExit) {
        setError(IndexerCrashed);
        setErrorText(QLatin1String("Indexer process crashed on "));// + m_file.url());
    }
    else if (exitCode == 1) {
        setError(IndexerFailed);
        //setErrorText(QLatin1String("Indexer process returned with an error for ") + m_file.url());
        /*
        if (FileIndexerConfig::self()->isDebugModeEnabled()) {
            QFile errorLogFile(KStandardDirs::locateLocal("data", QLatin1String("nepomuk/file-indexer-error-log"), true));
            if (errorLogFile.open(QIODevice::Append)) {
                QTextStream s(&errorLogFile);
                s << m_file.url() << ": " << QString::fromLocal8Bit(m_process->readAllStandardOutput()) << endl;
            }
        }
        */
    }

    emitResult();
}

void FileIndexingJob::slotProcessTimerTimeout()
{
    m_process->disconnect(this);
    m_process->kill();
    m_process->waitForFinished();
    setError(KJob::KilledJobError);
    setErrorText(QLatin1String("Indexer process got stuck for"));
    emitResult();
}

#include "fileindexingjob.moc"
