/* This file is part of the KDE Project
   Copyright (c) 2009-2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "metadatamover.h"
#include "nepomukfilewatch.h"
#include "datamanagement.h"
#include "resourcemanager.h"

#include <QtCore/QTimer>

#include <Soprano/Model>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>

#include "nie.h"

#include <KDebug>
#include <KJob>
#include <KLocale>

using namespace Nepomuk2::Vocabulary;

Nepomuk2::MetadataMover::MetadataMover(Soprano::Model* model, QObject* parent)
    : QObject(parent),
      m_queueMutex(QMutex::Recursive),
      m_model(model)
{
    // setup the main update queue timer
    m_queueTimer = new QTimer(this);
    connect(m_queueTimer, SIGNAL(timeout()),
            this, SLOT(slotWorkUpdateQueue()),
            Qt::DirectConnection);

}


Nepomuk2::MetadataMover::~MetadataMover()
{
}


void Nepomuk2::MetadataMover::moveFileMetadata(const KUrl& from, const KUrl& to)
{
//    kDebug() << from << to;
    Q_ASSERT(!from.path().isEmpty() && from.path() != "/");
    Q_ASSERT(!to.path().isEmpty() && to.path() != "/");

    QMutexLocker lock(&m_queueMutex);

    UpdateRequest req(from, to);
    if (!m_updateQueue.contains(req))
        m_updateQueue.enqueue(req);

    QTimer::singleShot(0, this, SLOT(slotStartUpdateTimer()));
}


void Nepomuk2::MetadataMover::removeFileMetadata(const KUrl& file)
{
    Q_ASSERT(!file.path().isEmpty() && file.path() != "/");
    removeFileMetadata(KUrl::List() << file);
}


void Nepomuk2::MetadataMover::removeFileMetadata(const KUrl::List& files)
{
    kDebug() << files;
    QMutexLocker lock(&m_queueMutex);

    foreach(const KUrl & file, files) {
        UpdateRequest req(file);
        if (!m_updateQueue.contains(req))
            m_updateQueue.enqueue(req);
    }

    QTimer::singleShot(0, this, SLOT(slotStartUpdateTimer()));
}


void Nepomuk2::MetadataMover::slotWorkUpdateQueue()
{
    // lock for initial iteration
    QMutexLocker lock(&m_queueMutex);

    emit metadataUpdateStarted();

    // work the queue
    if (!m_updateQueue.isEmpty()) {
        UpdateRequest updateRequest = m_updateQueue.dequeue();

        // unlock after queue utilization
        lock.unlock();

//        kDebug() << "========================= handling" << updateRequest.source() << updateRequest.target();

        // an empty second url means deletion
        if (updateRequest.target().isEmpty()) {

            emit statusMessage(i18n("Remove metadata from %1", updateRequest.source().prettyUrl()));
            removeMetadata(updateRequest.source());
        } else {
            const KUrl from = updateRequest.source();
            const KUrl to = updateRequest.target();

            emit statusMessage(i18n("Move metadata from %1 to %2", from.prettyUrl(), to.prettyUrl()));

            // We do NOT get deleted messages for overwritten files! Thus, we
            // have to remove all metadata for overwritten files first.
            removeMetadata(to);

            // and finally update the old statements
            updateMetadata(from, to);
        }

//        kDebug() << "========================= done with" << updateRequest.source() << updateRequest.target();
    } else {
        //kDebug() << "All update requests handled. Stopping timer.";

        emit metadataUpdateStopped();
        m_queueTimer->stop();
    }
}


namespace
{
QUrl fetchUriFromUrl(const QUrl& nieUrl)
{
    if (nieUrl.isEmpty()) {
        return QUrl();
    }

    QString query = QString::fromLatin1("select ?r where { ?r nie:url %1 . } LIMIT 1")
                    .arg(Soprano::Node::resourceToN3(nieUrl));

    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery(query, Soprano::Query::QueryLanguageSparqlNoInference);
    if (it.next())
        return it[0].uri();

    return QUrl();
}
}

void Nepomuk2::MetadataMover::removeMetadata(const KUrl& url)
{
    if (url.isEmpty()) {
        kDebug() << "empty path. Looks like a bug somewhere...";
        return;
    }

    const QUrl uri = fetchUriFromUrl(url);
    if (uri.isEmpty())
        return;

    // When the url is a folder we do not remove the metadata for all
    // the files in that folder, since inotify gives us a delete event
    // for each of those files
    KJob* job = Nepomuk2::removeResources(QList<QUrl>() << uri);
    job->exec();
    if (job->error())
        kError() << job->errorString();
}


void Nepomuk2::MetadataMover::updateMetadata(const KUrl& from, const KUrl& to)
{
    kDebug() << from << "->" << to;
    if (from.isEmpty() || to.isEmpty()) {
        kError() << "Paths Empty - File a bug" << from << to;
        return;
    }

    const QUrl uri = fetchUriFromUrl(from);
    if (!uri.isEmpty()) {
        KJob* job = Nepomuk2::setProperty(QList<QUrl>() << uri, NIE::url(), QVariantList() << to);
        job->exec();
        if (job->error())
            kError() << job->errorString();
    } else {
        //
        // If we have no metadata yet we need to tell the file indexer (if running) so it can
        // create the metadata in case the target folder is configured to be indexed.
        //
        emit movedWithoutData(to.path());
    }
}



// start the timer in the update thread
void Nepomuk2::MetadataMover::slotStartUpdateTimer()
{
    if (!m_queueTimer->isActive()) {
        m_queueTimer->start();
    }
}

#include "metadatamover.moc"
