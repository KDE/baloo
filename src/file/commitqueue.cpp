/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "commitqueue.h"
#include "lucenedocument.h"

#include <QDebug>
#include <KDiskFreeSpaceInfo>
#include <QCoreApplication>

Baloo::CommitQueue::CommitQueue(LuceneIndex *index, QObject* parent)
    : QObject(parent)
    , m_index(index)
{
    m_smallTimer.setSingleShot(true);
    m_smallTimer.setInterval(200);
    connect(&m_smallTimer, &QTimer::timeout, this, &CommitQueue::commit);

    m_largeTimer.setSingleShot(true);
    m_largeTimer.setInterval(10000);
    connect(&m_largeTimer, &QTimer::timeout, this, &CommitQueue::commit);
}

Baloo::CommitQueue::~CommitQueue()
{
    commit();
}

bool Baloo::CommitQueue::isEmpty() const
{
    return !m_index->haveChanges();
}

void Baloo::CommitQueue::add(unsigned id, Lucene::DocumentPtr doc)
{
    if (id) {
        LuceneDocument lDoc(doc);
        QString url = lDoc.getFieldValues("URL").at(0);
        m_index->replaceDocument(url, doc);
    } else {
        m_index->addDocument(doc);
    }
    startTimers();
}

void Baloo::CommitQueue::remove(const QString& url)
{
    m_index->deleteDocument(url);
    startTimers();
}

void Baloo::CommitQueue::startTimers()
{
    m_smallTimer.start();
    if (!m_largeTimer.isActive()) {
        m_largeTimer.start();
    }
}


void Baloo::CommitQueue::commit()
{
    // The 200 mb is arbitrary
    KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(m_index->path());
    if (info.isValid() && info.available() <= 200 * 1024 * 1024) {
        qWarning() << "Low disk space. Aborting!!";
        QCoreApplication::instance()->quit();
        return;
    }

    m_index->commit();

    m_smallTimer.stop();
    m_largeTimer.stop();

    Q_EMIT committed();
}
