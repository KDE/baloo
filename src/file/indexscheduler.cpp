/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2013 Vishesh Handa <handa.vish@gmail.com>

   Parts of this file are based on code from Strigi
   Copyright (C) 2006-2007 Jos van den Oever <jos@vandenoever.info>

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

#include "indexscheduler.h"
#include "fileindexerconfig.h"
#include "fileindexingqueue.h"
#include "basicindexingqueue.h"
#include "commitqueue.h"
#include "eventmonitor.h"
#include "database.h"

#include <KDebug>
#include <KLocale>

using namespace Baloo;

IndexScheduler::IndexScheduler(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_indexing(false)
    , m_config(config)
    , m_db(db)
{
    Q_ASSERT(m_config);
    connect(m_config, SIGNAL(includeFolderListChanged(QStringList, QStringList)),
            this, SLOT(slotIncludeFolderListChanged(QStringList, QStringList)));
    connect(m_config, SIGNAL(excludeFolderListChanged(QStringList, QStringList)),
            this, SLOT(slotExcludeFolderListChanged(QStringList, QStringList)));

    // FIXME: What if both the signals are emitted?
    connect(m_config, SIGNAL(fileExcludeFiltersChanged()),
            this, SLOT(slotConfigFiltersChanged()));
    connect(m_config, SIGNAL(mimeTypeFiltersChanged()),
            this, SLOT(slotConfigFiltersChanged()));

    // Stop indexing when a device is unmounted
    // RemovableMediaCache* cache = new RemovableMediaCache(this);
    // connect(cache, SIGNAL(deviceTeardownRequested(const RemovableMediaCache::Entry*)),
    //        this, SLOT(slotTeardownRequested(const RemovableMediaCache::Entry*)));

    m_basicIQ = new BasicIndexingQueue(m_db, m_config, this);
    m_fileIQ = new FileIndexingQueue(m_db, this);

    connect(m_basicIQ, SIGNAL(finishedIndexing()), this, SIGNAL(basicIndexingDone()));
    connect(m_fileIQ, SIGNAL(finishedIndexing()), this, SIGNAL(fileIndexingDone()));

    connect(m_basicIQ, SIGNAL(startedIndexing()), this, SLOT(slotStartedIndexing()));
    connect(m_basicIQ, SIGNAL(finishedIndexing()), this, SLOT(slotFinishedIndexing()));
    connect(m_fileIQ, SIGNAL(startedIndexing()), this, SLOT(slotStartedIndexing()));
    connect(m_fileIQ, SIGNAL(finishedIndexing()), this, SLOT(slotFinishedIndexing()));

    // Status String
    connect(m_basicIQ, SIGNAL(startedIndexing()), this, SLOT(emitStatusStringChanged()));
    connect(m_basicIQ, SIGNAL(finishedIndexing()), this, SLOT(emitStatusStringChanged()));
    connect(m_fileIQ, SIGNAL(startedIndexing()), this, SLOT(emitStatusStringChanged()));
    connect(m_fileIQ, SIGNAL(finishedIndexing()), this, SLOT(emitStatusStringChanged()));
    connect(this, SIGNAL(indexingSuspended(bool)), this, SLOT(emitStatusStringChanged()));

    m_eventMonitor = new EventMonitor(this);
    connect(m_eventMonitor, SIGNAL(idleStatusChanged(bool)),
            this, SLOT(slotScheduleIndexing()));
    connect(m_eventMonitor, SIGNAL(powerManagementStatusChanged(bool)),
            this, SLOT(slotScheduleIndexing()));

    m_commitQ = new CommitQueue(m_db, this);
    connect(m_commitQ, SIGNAL(committed()), this, SLOT(slotCommitted()));
    connect(m_basicIQ, SIGNAL(newDocument(uint,Xapian::Document)),
            m_commitQ, SLOT(add(uint,Xapian::Document)));
    connect(m_fileIQ, SIGNAL(newDocument(uint,Xapian::Document)),
            m_commitQ, SLOT(add(uint,Xapian::Document)));

    m_state = State_Normal;
    slotScheduleIndexing();
}


IndexScheduler::~IndexScheduler()
{
}


void IndexScheduler::suspend()
{
    if (m_state != State_Suspended) {
        m_state = State_Suspended;
        slotScheduleIndexing();

        m_eventMonitor->disable();
        Q_EMIT indexingSuspended(true);
    }
}


void IndexScheduler::resume()
{
    if (m_state == State_Suspended) {
        m_state = State_Normal;
        slotScheduleIndexing();

        m_eventMonitor->enable();
        Q_EMIT indexingSuspended(false);
    }
}


void IndexScheduler::setSuspended(bool suspended)
{
    if (suspended)
        suspend();
    else
        resume();
}

bool IndexScheduler::isSuspended() const
{
    return m_state == State_Suspended;
}

bool IndexScheduler::isIndexing() const
{
    return m_indexing;
}

void IndexScheduler::setIndexingStarted(bool started)
{
    if (started != m_indexing) {
        m_indexing = started;
        Q_EMIT indexingStateChanged(m_indexing);
        if (m_indexing)
            Q_EMIT indexingStarted();
        else
            Q_EMIT indexingStopped();
    }
}

void IndexScheduler::slotStartedIndexing()
{
    m_eventMonitor->enable();
    setIndexingStarted(true);
}

void IndexScheduler::slotFinishedIndexing()
{
    if (m_basicIQ->isEmpty()) {
        m_fileIQ->fillQueue();
        if (!m_fileIQ->isEmpty())
            slotScheduleIndexing();
    }

    if (m_basicIQ->isEmpty() && m_fileIQ->isEmpty()) {
        setIndexingStarted(false);
    }
}

void IndexScheduler::updateDir(const QString& path, UpdateDirFlags flags)
{
    m_basicIQ->enqueue(FileMapping(path), flags);
    slotScheduleIndexing();
}


void IndexScheduler::updateAll(bool forceUpdate)
{
    queueAllFoldersForUpdate(forceUpdate);
}


void IndexScheduler::queueAllFoldersForUpdate(bool forceUpdate)
{
    m_basicIQ->clear();

    UpdateDirFlags flags = UpdateRecursive | AutoUpdateFolder;
    if (forceUpdate)
        flags |= ForceUpdate;

    // update everything again in case the folders changed
    Q_FOREACH (const QString& f, m_config->includeFolders()) {
        m_basicIQ->enqueue(FileMapping(f), flags);
    }

    // Required to switch off the FileIQ
    slotScheduleIndexing();
}


void IndexScheduler::slotIncludeFolderListChanged(const QStringList& added, const QStringList& removed)
{
    //Index the folders added to the include list, clear the folders removed from it
    addClearFolders(added, removed);
}

void IndexScheduler::slotExcludeFolderListChanged(const QStringList& added, const QStringList& removed)
{
    //Clear the folders added to the exclude list, index the folders removed from it
    addClearFolders(removed, added);
}

//Index the folders in add, clear the folders in clear
void IndexScheduler::addClearFolders(const QStringList& add, const QStringList& clear)
{
    kDebug() << "To index: " << add << "To clear: " << clear;
    Q_FOREACH (const QString& path, clear) {
        m_basicIQ->clear(path);
        m_fileIQ->clear();
    }

    Q_FOREACH (const QString &path, add) {
        m_basicIQ->enqueue(FileMapping(path), UpdateRecursive);
    }
    slotScheduleIndexing();
}

void IndexScheduler::slotConfigFiltersChanged()
{
    // We need to this - there is no way to avoid it
    m_basicIQ->clear();
    m_fileIQ->clear();

    queueAllFoldersForUpdate();
}


void IndexScheduler::indexFile(const QString& path)
{
    m_basicIQ->enqueue(FileMapping(path));
}

/*
void IndexScheduler::slotTeardownRequested(const RemovableMediaCache::Entry* entry)
{
    const QString path = entry->mountPath();

    m_basicIQ->clear(path);
    m_fileIQ->clear(path);
}
*/


void IndexScheduler::setStateFromEvent()
{
   //Don't change the state if already suspended
    if (m_state == State_Suspended) {
        kDebug() << "Suspended";
    }
    else if (m_eventMonitor->isOnBattery()) {
        kDebug() << "Battery";
        m_state = State_OnBattery;
    }
    else if (m_eventMonitor->isIdle()) {
        kDebug() << "Idle";
        m_state = State_UserIdle;
    }
    else {
        kDebug() << "Normal";
        m_state = State_Normal;
    }
}

bool IndexScheduler::scheduleBasicQueue()
{
    switch (m_state) {
        case State_Suspended:
            kDebug() << "No basic queue: suspended";
            return false;
        case State_OnBattery:
        case State_UserIdle:
        case State_Normal:
        default:
            return true;
    }
}


bool IndexScheduler::scheduleFileQueue()
{
    if (!m_basicIQ->isEmpty()){
        kDebug() << "Basic queue not empty, so no file queue.";
        return false;
    }
    switch (m_state) {
        case State_Suspended:
        case State_OnBattery:
            kDebug() << "No file queue: suspended or on battery";
            return false;
        case State_UserIdle:
        case State_Normal:
        default:
            return true;
    }
}

void IndexScheduler::slotScheduleIndexing()
{
    //Set the state from the event monitor
    setStateFromEvent();

    //Should we run the basic queue?
    bool runBasic = scheduleBasicQueue();

    //If we should not, stop.
    if (!runBasic) {
        m_basicIQ->suspend();
        m_fileIQ->suspend();
    }
    else {
        //Run the basic queue if it isn't empty
        if (!m_basicIQ->isEmpty()) {
            m_basicIQ->setDelay(0);
            m_basicIQ->resume();
        }
        //Consider running the file queue:
        //this will only happen if the basic queue is not empty.
        if (scheduleFileQueue())
            m_fileIQ->resume();
        else
            m_fileIQ->suspend();
    }
}

QString IndexScheduler::userStatusString() const
{
    bool indexing = isIndexing();
    bool suspended = isSuspended();
    bool processing = !m_basicIQ->isEmpty();

    if (suspended) {
        return i18nc("@info:status", "File indexer is suspended.");
    } else if (indexing) {
        return i18nc("@info:status", "Indexing files for desktop search.");
    } else if (processing) {
        return i18nc("@info:status", "Scanning for recent changes in files for desktop search");
    } else {
        return i18nc("@info:status", "File indexer is idle.");
    }
}

IndexScheduler::State IndexScheduler::currentStatus() const
{
    return m_state;
}

void IndexScheduler::emitStatusStringChanged()
{
    QString status = userStatusString();
    if (status != m_oldStatus) {
        Q_EMIT statusStringChanged();
        m_oldStatus = status;
    }
}

void IndexScheduler::slotCommitted()
{
    if (m_basicIQ->isEmpty()) {
        m_fileIQ->resume();
    }
}

void IndexScheduler::removeFileData(int id)
{
    m_commitQ->remove(id);
}
