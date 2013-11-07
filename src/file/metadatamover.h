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

#ifndef _NEPOMUK_METADATA_MOVER_H_
#define _NEPOMUK_METADATA_MOVER_H_

#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QSet>
#include <QtCore/QDateTime>

#include <KUrl>

#include "updaterequest.h"

class QTimer;

namespace Soprano
{
class Model;
}

namespace Nepomuk2
{
class MetadataMover : public QObject
{
    Q_OBJECT

public:
    MetadataMover(Soprano::Model* model, QObject* parent = 0);
    ~MetadataMover();

public Q_SLOTS:
    void moveFileMetadata(const KUrl& from, const KUrl& to);
    void removeFileMetadata(const KUrl& file);
    void removeFileMetadata(const KUrl::List& files);

Q_SIGNALS:
    /**
     * Emitted for files (and folders) that have been moved but
     * do not have metadata to be moved. This allows the file indexer
     * service to pick them up in case they are of interest. The
     * typical example would be moving a file from a non-indexed into
     * an indexed folder.
     */
    void movedWithoutData(const QString& path);

    /**
     * @brief Emitted once for each remove and/or move requests
     *
     * @param msg translated msg message indicating what teh watcher is doing
     */
    void statusMessage(const QString& msg);

    /**
     * @brief Emitted once remove/move requests are started
     */
    void metadataUpdateStarted();

    /**
     * @brief Emitted once all remove/move requests are finished
     */
    void metadataUpdateStopped();

private Q_SLOTS:
    void slotWorkUpdateQueue();

    /**
     * Start the update queue from the main thread.
     */
    void slotStartUpdateTimer();

private:
    /**
     * Remove the metadata for file \p url
     */
    void removeMetadata(const KUrl& url);

    /**
     * Recursively update the nie:url and nie:isPartOf properties
     * of the resource describing \p from.
     */
    void updateMetadata(const KUrl& from, const KUrl& to);

    // if the second url is empty, just delete the metadata
    QQueue<UpdateRequest> m_updateQueue;

    QMutex m_queueMutex;

    QTimer* m_queueTimer;

    Soprano::Model* m_model;
};
}

#endif
