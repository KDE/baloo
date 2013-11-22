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
#include "filemapping.h"
#include "database.h"

#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>
#include <KJob>
#include <KLocale>

using namespace Baloo;

MetadataMover::MetadataMover(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_queueMutex(QMutex::Recursive)
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


void MetadataMover::moveFileMetadata(const QString& from, const QString& to)
{
//    kDebug() << from << to;
    Q_ASSERT(!from.isEmpty() && from != "/");
    Q_ASSERT(!to.isEmpty() && to != "/");

    QMutexLocker lock(&m_queueMutex);

    UpdateRequest req(from, to);
    if (!m_updateQueue.contains(req))
        m_updateQueue.enqueue(req);

    QTimer::singleShot(0, this, SLOT(slotStartUpdateTimer()));
}


void MetadataMover::removeFileMetadata(const QString& file)
{
    Q_ASSERT(!file.isEmpty() && file != "/");
    removeFileMetadata(QStringList() << file);
}


void MetadataMover::removeFileMetadata(const QStringList& files)
{
    kDebug() << files;
    QMutexLocker lock(&m_queueMutex);

    Q_FOREACH (const QString& file, files) {
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

            Q_EMIT statusMessage(i18n("Remove metadata from %1", updateRequest.source()));
            removeMetadata(updateRequest.source());
        } else {
            const QString from = updateRequest.source();
            const QString to = updateRequest.target();

            Q_EMIT statusMessage(i18n("Move metadata from %1 to %2", from, to));

            // We do NOT get deleted messages for overwritten files! Thus, we
            // have to remove all metadata for overwritten files first.
            removeMetadata(to);

            // and finally update the old statements
            updateMetadata(from, to);
        }

//        kDebug() << "========================= done with" << updateRequest.source() << updateRequest.target();
    } else {
        //kDebug() << "All update requests handled. Stopping timer.";

        m_db->sqlDatabase().commit();
        m_db->sqlDatabase().transaction();

        Q_EMIT metadataUpdateStopped();
        m_queueTimer->stop();
    }
}


void MetadataMover::removeMetadata(const QString& url)
{
    if (url.isEmpty()) {
        kDebug() << "empty path. Looks like a bug somewhere...";
        return;
    }

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("delete from files where url = ?");
    query.addBindValue(url);
    if (!query.exec()) {
        kError() << query.lastError().text();
    }
}


void MetadataMover::updateMetadata(const QString& from, const QString& to)
{
    kDebug() << from << "->" << to;
    if (from.isEmpty() || to.isEmpty()) {
        kError() << "Paths Empty - File a bug" << from << to;
        return;
    }

    Q_ASSERT(from[from.size()-1] != '/');
    Q_ASSERT(to[to.size()-1] != '/');

    FileMapping fromFile(from);
    fromFile.fetch(m_db->sqlDatabase());

    if (fromFile.fetch(m_db->sqlDatabase())) {
        QSqlQuery q(m_db->sqlDatabase());
        q.prepare("update files set url = ? where id = ?");
        q.addBindValue(to);
        q.addBindValue(fromFile.id());
        if (!q.exec())
            kError() << q.lastError().text();
    }

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("update files set url = ':t' || substr(url, :fs) "
                  "where url like ':f/%'");

    query.bindValue(":t", to);
    query.bindValue(":fs", from.size());
    query.bindValue(":f", from);

    if (!query.exec()) {
        kError() << "Big query failed:" << query.lastError().text();
    }

    if (!fromFile.id()) {
        //
        // If we have no metadata yet we need to tell the file indexer (if running) so it can
        // create the metadata in case the target folder is configured to be indexed.
        //
        Q_EMIT movedWithoutData(to);
    }
}

// start the timer in the update thread
void MetadataMover::slotStartUpdateTimer()
{
    if (!m_queueTimer->isActive()) {
        m_queueTimer->start();
    }
}

#include "metadatamover.moc"
