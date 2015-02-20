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
#include "indexingleveldb.h"
#include "positiondb.h"

#include "document.h"

#include "andpostingiterator.h"
#include "documentvaluedb.h"

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
{
    QFileInfo dirInfo(path);
    if (!dirInfo.permission(QFile::WriteOwner)) {
        qCritical() << path << "does not have write permissions. Aborting";
        exit(1);
    }

    mdb_env_create(&m_env);
    mdb_env_set_maxdbs(m_env, 7);
    mdb_env_set_mapsize(m_env, 104857600);

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(path);
    mdb_env_open(m_env, arr.constData(), 0, 0664);
    mdb_txn_begin(m_env, NULL, 0, &m_txn);

    m_postingDB = new PostingDB(m_txn);
    m_documentDB = new DocumentDB(m_txn);
    m_positionDB = new PositionDB(m_txn);
    m_docUrlDB = new DocumentUrlDB(m_txn);
    m_urlDocDB = new UrlDocumentDB(m_txn);
    m_docValueDB = new DocumentValueDB(m_txn);
    m_indexingLevelDB = new IndexingLevelDB(m_txn);
}

Database::~Database()
{
    delete m_postingDB;
    delete m_documentDB;
    delete m_positionDB;
    delete m_docUrlDB;
    delete m_urlDocDB;
    delete m_docValueDB;
    delete m_indexingLevelDB;

    mdb_txn_commit(m_txn);
    mdb_env_close(m_env);
}

QString Database::path() const
{
    return m_path;
}

void Database::addDocument(const Document& doc)
{
    Q_ASSERT(doc.id() > 0);

    Operation op;
    op.type = AddDocument;
    op.doc = doc;

    m_pendingOperations << op;
}

void Database::removeDocument(uint id)
{
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
    Q_ASSERT(!url.isEmpty());
    return m_urlDocDB->get(url);
}

QByteArray Database::documentUrl(uint id)
{
    Q_ASSERT(id > 0);
    return m_docUrlDB->get(id);
}


QByteArray Database::documentSlot(uint id, uint slotNum)
{
    Q_ASSERT(id > 0);
    return m_docValueDB->get(id, slotNum);
}

void Database::commit()
{
    QMap<QByteArray, PostingList> postingMap;
    QMap<QByteArray, QVector<PositionInfo> > positionMap;

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

            if (doc.indexingLevel()) {
                m_indexingLevelDB->put(doc.id());
            }
            if (!doc.m_slots.isEmpty()) {
                for (auto it = doc.m_slots.constBegin(); it != doc.m_slots.constEnd(); it++) {
                    m_docValueDB->put(doc.id(), it.key(), it.value());
                }
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

            m_indexingLevelDB->del(id);
            m_docValueDB->del(id);
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
}

bool Database::hasChanges() const
{
    return !m_pendingOperations.isEmpty();
}

QVector<int> Database::exec(const QVector<QByteArray>& query)
{
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
        return QVector<int>();
    }

    QVector<int> result;
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
    return m_indexingLevelDB->fetchItems(size);
}
