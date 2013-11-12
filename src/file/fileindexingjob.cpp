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
#include "resourcemanager.h"
#include "kext.h"

#include <KUrl>
#include <KDebug>
#include <KProcess>
#include <KStandardDirs>

#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

using namespace Nepomuk2::Vocabulary;

Nepomuk2::FileIndexingJob::FileIndexingJob(const QUrl& fileUrl, QObject* parent)
    : KJob(parent),
      m_url(fileUrl)
{
    // setup the timer used to kill the indexer process if it seems to get stuck
    m_processTimer = new QTimer(this);
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, SIGNAL(timeout()),
            this, SLOT(slotProcessTimerTimeout()));
}

void Nepomuk2::FileIndexingJob::start()
{
    if (!QFile::exists(m_url.toLocalFile())) {
        QTimer::singleShot(0, this, SLOT(slotProcessNonExistingFile()));
        return;
    }

    // setup the external process which does the actual indexing
    const QString exe = KStandardDirs::findExe(QLatin1String("nepomukindexer"));

    kDebug() << "Running" << exe << m_url.toLocalFile();

    m_process = new KProcess(this);

    QStringList args;
    args << m_url.toLocalFile();

    m_process->setProgram(exe, args);
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotIndexedFile(int, QProcess::ExitStatus)));
    m_process->start();

    // start the timer which will kill the process if it does not terminate after 5 minutes
    m_processTimer->start(5 * 60 * 1000);
}

void Nepomuk2::FileIndexingJob::slotProcessNonExistingFile()
{
    QString query = QString::fromLatin1("select ?r where { ?r nie:url %1. }")
                    .arg(Soprano::Node::resourceToN3(m_url));
    Soprano::Model* model = ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);
    while (it.next()) {
        QUrl uri = it[0].uri();

        // We do not just delete the resource cause it could be part of some removeable media
        // which is not mounted. When the device is mounted, then the file will get reindexed
        model->removeAllStatements(uri, KExt::indexingLevel(), QUrl());
    }

    emitResult();
}


void Nepomuk2::FileIndexingJob::slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus)
{
    // stop the timer since there is no need to kill the process anymore
    m_processTimer->stop();

    //kDebug() << "Indexing of " << m_url.toLocalFile() << "finished with exit code" << exitCode;
    if (exitStatus != QProcess::NormalExit) {
        setError(IndexerCrashed);
        setErrorText(QLatin1String("Indexer process crashed on ") + m_url.toLocalFile());
    }
    if (exitCode == 1) {
        setError(IndexerFailed);
        setErrorText(QLatin1String("Indexer process returned with an error for ") + m_url.toLocalFile());
        if (FileIndexerConfig::self()->isDebugModeEnabled()) {
            QFile errorLogFile(KStandardDirs::locateLocal("data", QLatin1String("nepomuk/file-indexer-error-log"), true));
            if (errorLogFile.open(QIODevice::Append)) {
                QTextStream s(&errorLogFile);
                s << m_url.toLocalFile() << ": " << QString::fromLocal8Bit(m_process->readAllStandardOutput()) << endl;
            }
        }
    }
    emitResult();
}

void Nepomuk2::FileIndexingJob::slotProcessTimerTimeout()
{
    m_process->disconnect(this);
    m_process->kill();
    m_process->waitForFinished();
    setError(KJob::KilledJobError);
    setErrorText(QLatin1String("Indexer process got stuck for") + m_url.toLocalFile());
    emitResult();
}

#include "fileindexingjob.moc"
