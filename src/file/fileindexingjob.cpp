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

#include <KUrl>
#include <KDebug>
#include <KProcess>
#include <KStandardDirs>

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QDateTime>

using namespace Baloo;

FileIndexingJob::FileIndexingJob(Database* db, const FileMapping& file, QObject* parent)
    : KJob(parent)
    , m_db(db)
    , m_file(file)
{
    // setup the timer used to kill the indexer process if it seems to get stuck
    m_processTimer = new QTimer(this);
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, SIGNAL(timeout()),
            this, SLOT(slotProcessTimerTimeout()));
}

void FileIndexingJob::start()
{
    if (!QFile::exists(m_file.url())) {
        QTimer::singleShot(0, this, SLOT(slotProcessNonExistingFile()));
        return;
    }

    // setup the external process which does the actual indexing
    const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_extractor"));

    m_process = new QProcess(this);

    QStringList args;
    args << m_file.url();
    kDebug() << args;

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotIndexedFile(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStdOutput()));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->start(exe, args);

    // start the timer which will kill the process if it does not terminate after 5 minutes
    m_processTimer->start(5 * 60 * 1000);
}

void FileIndexingJob::slotProcessNonExistingFile()
{
    if (m_file.id())
        m_db->xapainDatabase()->delete_document(m_file.id());

    emitResult();
}

void FileIndexingJob::slotReadyReadStdOutput()
{
    QByteArray arr = QByteArray::fromBase64(m_process->readAll());
    QDataStream st(&arr, QIODevice::ReadOnly);

    QVariantMap map;
    st >> map;

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(map);

    m_file.fetch(m_db);

    Xapian::Document doc = m_db->xapainDatabase()->get_document(m_file.id());
    Xapian::TermGenerator termGen;
    termGen.set_document(doc);
    termGen.set_database(*m_db->xapainDatabase());

    // Save the data for fetching later
    doc.set_data(json.constData());

    // Index the data
    QMap<QString, QVariant>::const_iterator it = map.constBegin();
    for (; it != map.constEnd(); it++) {
        const QString key = it.key().toUpper();
        const QVariant val = it.value();

        if (val.type() == QVariant::Bool) {
            doc.add_boolean_term(key.toStdString());
        }
        else if (val.type() == QVariant::Int) {
            const QString term = key + val.toString();
            doc.add_term(term.toStdString());
        }
        else if (val.type() == QVariant::DateTime) {
            const QString term = key + val.toDateTime().toString(Qt::ISODate);
            doc.add_term(term.toStdString());
        }
        else {
            QString term = key;
            if (term == "TEXT")
                term.clear();

            termGen.index_text(val.toString().toStdString(), 1, term.toStdString());
        }
    }
}


void FileIndexingJob::slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus)
{
    // stop the timer since there is no need to kill the process anymore
    m_processTimer->stop();

    //kDebug() << "Indexing of " << m_url.toLocalFile() << "finished with exit code" << exitCode;
    if (exitStatus != QProcess::NormalExit) {
        setError(IndexerCrashed);
        setErrorText(QLatin1String("Indexer process crashed on ") + m_file.url());
    }
    else if (exitCode == 1) {
        setError(IndexerFailed);
        setErrorText(QLatin1String("Indexer process returned with an error for ") + m_file.url());
        if (FileIndexerConfig::self()->isDebugModeEnabled()) {
            QFile errorLogFile(KStandardDirs::locateLocal("data", QLatin1String("nepomuk/file-indexer-error-log"), true));
            if (errorLogFile.open(QIODevice::Append)) {
                QTextStream s(&errorLogFile);
                s << m_file.url() << ": " << QString::fromLocal8Bit(m_process->readAllStandardOutput()) << endl;
            }
        }
    }
    else {
        kDebug() << "UPDATING IL to 2";
        updateIndexingLevel(m_db, m_file.id(), 2);
    }

    emitResult();
}

void FileIndexingJob::slotProcessTimerTimeout()
{
    m_process->disconnect(this);
    m_process->kill();
    m_process->waitForFinished();
    setError(KJob::KilledJobError);
    setErrorText(QLatin1String("Indexer process got stuck for") + m_file.url());
    emitResult();
}

#include "fileindexingjob.moc"
