/*
   This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "database.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "urldocumentdb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documentvaluedb.h"
#include "documentdatadb.h"

#include "document.h"

#include "andpostingiterator.h"

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(0)
    , m_txn(0)
    , m_postingDB(0)
    , m_documentDB(0)
    , m_positionDB(0)
    , m_docUrlDB(0)
    , m_urlDocDB(0)
    , m_docValueDB(0)
    , m_docDataDB(0)
    , m_contentIndexingDB(0)
{
}

Database::~Database()
{
    delete m_postingDB;
    delete m_documentDB;
    delete m_positionDB;
    delete m_docUrlDB;
    delete m_urlDocDB;
    delete m_docValueDB;
    delete m_docDataDB;
    delete m_contentIndexingDB;

    if (m_txn) {
        abort();
    }
    mdb_env_close(m_env);
}

bool Database::open()
{
    QFileInfo dirInfo(m_path);
    if (!dirInfo.permission(QFile::WriteOwner)) {
        qCritical() << m_path << "does not have write permissions. Aborting";
        return false;
    }

    mdb_env_create(&m_env);
    mdb_env_set_maxdbs(m_env, 8);
    mdb_env_set_mapsize(m_env, 1048576000);

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(m_path);
    mdb_env_open(m_env, arr.constData(), 0, 0664);

    return true;
}

void Database::transaction()
{
    Q_ASSERT(m_txn == 0);
    int rc = mdb_txn_begin(m_env, NULL, 0, &m_txn);
    Q_ASSERT_X(rc == 0, "Database::transaction", mdb_strerror(rc));

    if (!m_positionDB)
        m_postingDB = new PostingDB(m_txn);
    else
        m_positionDB->setTransaction(m_txn);

    if (!m_documentDB)
        m_documentDB = new DocumentDB(m_txn);
    else
        m_documentDB->setTransaction(m_txn);

    if (!m_positionDB)
        m_positionDB = new PositionDB(m_txn);
    else
        m_positionDB->setTransaction(m_txn);

    if (!m_docUrlDB)
        m_docUrlDB = new DocumentUrlDB(m_txn);
    else
        m_docUrlDB->setTransaction(m_txn);

    if (!m_urlDocDB)
        m_urlDocDB = new UrlDocumentDB(m_txn);
    else
        m_urlDocDB->setTransaction(m_txn);

    if (!m_docValueDB)
        m_docValueDB = new DocumentValueDB(m_txn);
    else
        m_docUrlDB->setTransaction(m_txn);

    if (!m_docDataDB)
        m_docDataDB = new DocumentDataDB("documentdatadb", m_txn);
    else
        m_docDataDB->setTransaction(m_txn);

    if (!m_contentIndexingDB)
        m_contentIndexingDB = new DocumentIdDB(m_txn);
    else
        m_contentIndexingDB->setTransaction(m_txn);
}

QString Database::path() const
{
    return m_path;
}

void Database::addDocument(const Document& doc)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);

    Operation op;
    op.type = AddDocument;
    op.doc = doc;

    m_pendingOperations << op;
}

void Database::removeDocument(uint id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    Document doc;
    doc.setId(id);

    Operation op;
    op.type = AddDocument;
    op.doc = doc;

    m_pendingOperations << op;
}

bool Database::hasDocument(uint id)
{
    Q_ASSERT(id > 0);
    return m_documentDB->contains(id);
}

uint Database::documentId(const QByteArray& url)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(!url.isEmpty());
    return m_urlDocDB->get(url);
}

QByteArray Database::documentUrl(uint id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    return m_docUrlDB->get(id);
}


QByteArray Database::documentSlot(uint id, uint slotNum)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    return m_docValueDB->get(id, slotNum);
}

QByteArray Database::documentData(uint id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    return m_docDataDB->get(id);
}

void Database::commit()
{
    Q_ASSERT(m_txn);
    QMap<QByteArray, PostingList> postingMap;
    QMap<QByteArray, QVector<PositionInfo> > positionMap;

    qDebug() << "PendingOperations:" << m_pendingOperations.size();
    for (const Operation& op : m_pendingOperations) {
        const uint id = op.doc.id();
        const Document& doc = op.doc;

        if (op.type == AddDocument) {
            QVector<QByteArray> docTerms;
            docTerms.reserve(doc.m_terms.size());

            QMapIterator<QByteArray, Document::TermData> it(doc.m_terms);
            while (it.hasNext()) {
                const QByteArray term = it.next().key();
                docTerms.append(term);

                postingMap[term] << id;

                const Document::TermData td = it.value();
                if (!td.positions.isEmpty()) {
                    PositionInfo pi(id, it.value().positions);
                    positionMap[term] << pi;
                }
            }

            m_documentDB->put(id, docTerms);

            if (!doc.url().isEmpty()) {
                m_docUrlDB->put(id, doc.url());
                m_urlDocDB->put(doc.url(), id);
            }

            if (doc.contentIndexing()) {
                m_contentIndexingDB->put(doc.id());
            }
            if (!doc.m_slots.isEmpty()) {
                for (auto iter = doc.m_slots.constBegin(); iter != doc.m_slots.constEnd(); iter++) {
                    m_docValueDB->put(doc.id(), iter.key(), iter.value());
                }
            }

            if (!doc.m_data.isEmpty()) {
                m_docDataDB->put(id, doc.m_data);
            }
        }
        else if (op.type == RemoveDocument) {
            QVector<QByteArray> terms = m_documentDB->get(id);
            if (terms.isEmpty()) {
                return;
            }

            for (const QByteArray& term : terms) {
                // FIXME: The will not update stuff correctly!
                PostingList list = m_postingDB->get(term);
                list.removeOne(id);

                m_postingDB->put(term, list);

                QVector<PositionInfo> posInfoList = m_positionDB->get(term);
                posInfoList.removeOne(PositionInfo(id));
            }

            m_documentDB->del(id);

            QByteArray url = m_docUrlDB->get(id);
            m_docUrlDB->del(id);
            m_urlDocDB->del(url);

            m_contentIndexingDB->del(id);
            m_docValueDB->del(id);
            m_docDataDB->del(id);
        }
    }

    m_pendingOperations.clear();

    //
    // Process postingList and positionList
    //
    QMapIterator<QByteArray, PostingList> it(postingMap);
    while (it.hasNext()) {
        it.next();
        const QByteArray term = it.key();

        PostingList list = m_postingDB->get(term);
        list << it.value();

        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());

        m_postingDB->put(term, list);
    }

    QMapIterator<QByteArray, QVector<PositionInfo> > iter(positionMap);
    while (iter.hasNext()) {
        iter.next();
        const QByteArray term = iter.key();

        QVector<PositionInfo> list = m_positionDB->get(term);
        list << iter.value();

        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());

        m_positionDB->put(term, list);
    }

    int rc = mdb_txn_commit(m_txn);
    Q_ASSERT_X(rc == 0, "Database::commit", mdb_strerror(rc));

    m_txn = 0;
}

void Database::abort()
{
    Q_ASSERT(m_txn);

    mdb_txn_abort(m_txn);
    m_txn = 0;
}

bool Database::hasChanges() const
{
    Q_ASSERT(m_txn);
    return !m_pendingOperations.isEmpty();
}

QVector<uint> Database::exec(const QVector<QByteArray>& query)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(!query.isEmpty());

    QVector<PostingIterator*> list;
    list.reserve(query.size());

    for (const QByteArray& term : query) {
        PostingIterator* iter = m_postingDB->iter(term);
        if (iter) {
            list << iter;
        }
    }

    if (list.isEmpty()) {
        return QVector<uint>();
    }

    QVector<uint> result;
    if (list.size() == 1) {
        PostingIterator* it = list.first();
        while (it->next()) {
            result << it->docId();
        }
        return result;
    }

    AndPostingIterator it(list);
    while (it.next()) {
        result << it.docId();
    }

    return result;
}

QVector<uint> Database::fetchIndexingLevel(int size)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(size > 0);
    return m_contentIndexingDB->fetchItems(size);
}
