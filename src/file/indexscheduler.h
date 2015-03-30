/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012 Vishesh Handa <me@vhanda.in>

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

#ifndef BALOO_FILEINDEXER_INDEX_SCHEDULER_H_
#define BALOO_FILEINDEXER_INDEX_SCHEDULER_H_

#include "basicindexingqueue.h" // Required for UpdateDirFlags

namespace Baloo
{

class Database;
class FileIndexingQueue;
class FileIndexerConfig;
class CommitQueue;
class EventMonitor;
class SchedulerTest;

/**
 * The IndexScheduler is responsible for controlling the indexing
 * queues and reacting to events. It contains an EventMonitor
 * and listens for events such as power management, battery and
 * disk space.
 */
class IndexScheduler : public QObject
{
    Q_OBJECT

public:
    IndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent = 0);
    ~IndexScheduler();

    bool isSuspended() const;
    bool isIndexing() const;

    /**
     * A user readable description of the scheduler's status
     */
    QString userStatusString() const;

public Q_SLOTS:
    void suspend();
    void resume();

    void setSuspended(bool);

    /**
     * Slot to connect to certain event systems like KDirNotify
     * or KDirWatch
     *
     * Updates a complete folder. Makes sense for
     * signals like KDirWatch::dirty.
     *
     * \param path The folder to update
     * \param flags Additional flags, all except AutoUpdateFolder are supported. This
     * also means that by default \p path is updated non-recursively.
     */
    void updateDir(const QString& path, UpdateDirFlags flags = NoUpdateFlags);

    /**
     * Updates all configured folders.
     */
    void updateAll(bool forceUpdate = false);

    /**
     * Send this specific file for indexing
     */
    void indexFile(const QString& path);

    /**
     * Send the url for indexing, but only index the extended attributes
     * of the file
     */
    void indexXattr(const QString& path);

Q_SIGNALS:
    // Indexing State
    void indexingStarted();
    void indexingStopped();
    void indexingStateChanged(bool indexing);

    void basicIndexingDone();
    void fileIndexingDone();

    // Emitted on calling suspend/resume
    void indexingSuspended(bool suspended);

    void statusStringChanged();
private Q_SLOTS:
    // Config
    void slotConfigChanged();

    void slotStartedIndexing();

    // Event Monitor integration
    void slotScheduleIndexing();

    void emitStatusStringChanged();
    void slotNotifyCommitted();

private:
    void queueAllFoldersForUpdate(bool forceUpdate = false);

    // emits indexingStarted or indexingStopped based on parameter. Makes sure
    // no signal is emitted twice
    void setIndexingStarted(bool started);

    bool shouldRunBasicQueue();
    bool shouldRunFileQueue();
    void setStateFromEvent();

    bool m_indexing;

    FileIndexerConfig* m_config;

    // Queues
    BasicIndexingQueue* m_basicIQ;
    FileIndexingQueue* m_fileIQ;
    CommitQueue* m_commitQ;

    EventMonitor* m_eventMonitor;

    enum State {
        State_Normal = 0,
        State_UserIdle = 1,
        State_OnBattery = 2,
        State_Suspended = 3
    };
    State m_state;

    QString m_oldStatus;

    Database* m_db;

    friend class SchedulerTest;
};
}

#endif
