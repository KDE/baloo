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
#include "filewatch.h"

#include <QtCore/QTimer>

#include <KDebug>
#include <KJob>
#include <KLocale>

using namespace Baloo;

MetadataMover::MetadataMover(QObject* parent)
    : QObject(parent),
      m_queueMutex(QMutex::Recursive)
{
    // setup the main update queue timer
    m_queueTimer = new QTimer(this);
    connect(m_queueTimer, SIGNAL(timeout()),
            this, SLOT(slotWorkUpdateQueue()),
            Qt::DirectConnection);

}


MetadataMover::~MetadataMover()
{
}


void MetadataMover::moveFileMetadata(const KUrl& from, const KUrl& to)
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


void MetadataMover::removeFileMetadata(const KUrl& file)
{
    Q_ASSERT(!file.path().isEmpty() && file.path() != "/");
    removeFileMetadata(KUrl::List() << file);
}


void MetadataMover::removeFileMetadata(const KUrl::List& files)
{
    kDebug() << files;
    QMutexLocker lock(&m_queueMutex);

    Q_FOREACH (const KUrl& file, files) {
        UpdateRequest req(file);
        if (!m_updateQueue.contains(req))
            m_updateQueue.enqueue(req);
    }

    QTimer::singleShot(0, this, SLOT(slotStartUpdateTimer()));
}


void MetadataMover::slotWorkUpdateQueue()
{
    // lock for initial iteration
    QMutexLocker lock(&m_queueMutex);

    Q_EMIT metadataUpdateStarted();

    // work the queue
    if (!m_updateQueue.isEmpty()) {
        UpdateRequest updateRequest = m_updateQueue.dequeue();

        // unlock after queue utilization
        lock.unlock();

//        kDebug() << "========================= handling" << updateRequest.source() << updateRequest.target();

        // an empty second url means deletion
        if (updateRequest.target().isEmpty()) {

            Q_EMIT statusMessage(i18n("Remove metadata from %1", updateRequest.source().prettyUrl()));
            removeMetadata(updateRequest.source());
        } else {
            const KUrl from = updateRequest.source();
            const KUrl to = updateRequest.target();

            Q_EMIT statusMessage(i18n("Move metadata from %1 to %2", from.prettyUrl(), to.prettyUrl()));

            // We do NOT get deleted messages for overwritten files! Thus, we
            // have to remove all metadata for overwritten files first.
            removeMetadata(to);

            // and finally update the old statements
            updateMetadata(from, to);
        }

//        kDebug() << "========================= done with" << updateRequest.source() << updateRequest.target();
    } else {
        //kDebug() << "All update requests handled. Stopping timer.";

        Q_EMIT metadataUpdateStopped();
        m_queueTimer->stop();
    }
}


void MetadataMover::removeMetadata(const KUrl& url)
{
    if (url.isEmpty()) {
        kDebug() << "empty path. Looks like a bug somewhere...";
        return;
    }

#warning FIXME: needs a fetchUriFromUrl
    const QUrl uri;
    if (uri.isEmpty())
        return;

    // When the url is a folder we do not remove the metadata for all
    // the files in that folder, since inotify gives us a delete event
    // for each of those files
#warning FIXME: There is no removeResources job
/*    KJob* job = Nepomuk2::removeResources(QList<QUrl>() << uri);
    job->exec();
    if (job->error())
        kError() << job->errorString();
    */
}


void MetadataMover::updateMetadata(const KUrl& from, const KUrl& to)
{
    kDebug() << from << "->" << to;
    if (from.isEmpty() || to.isEmpty()) {
        kError() << "Paths Empty - File a bug" << from << to;
        return;
    }

    /*
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
    }*/
}



// start the timer in the update thread
void MetadataMover::slotStartUpdateTimer()
{
    if (!m_queueTimer->isActive()) {
        m_queueTimer->start();
    }
}

#include "metadatamover.moc"
