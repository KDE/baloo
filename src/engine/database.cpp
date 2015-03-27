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
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"

#include "document.h"
#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "idutils.h"
#include "termgenerator.h"

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(0)
    , m_txn(0)
    , m_postingDB(0)
    , m_positionDB(0)
    , m_documentTermsDB(0)
    , m_documentXattrTermsDB(0)
    , m_documentFileNameTermsDB(0)
    , m_docUrlDB(0)
    , m_docTimeDB(0)
    , m_docDataDB(0)
    , m_contentIndexingDB(0)
{
}

Database::~Database()
{
    delete m_postingDB;
    delete m_positionDB;
    delete m_documentTermsDB;
    delete m_documentXattrTermsDB;
    delete m_documentFileNameTermsDB;
    delete m_docUrlDB;
    delete m_docTimeDB;
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
    mdb_env_set_maxdbs(m_env, 9);
    mdb_env_set_mapsize(m_env, 1048576000);

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(m_path);
    mdb_env_open(m_env, arr.constData(), 0, 0664);

    return true;
}

void Database::transaction(Database::TransactionType type)
{
    Q_ASSERT(m_txn == 0);

    uint flags = type == ReadOnly ? MDB_RDONLY : 0;
    int rc = mdb_txn_begin(m_env, NULL, flags, &m_txn);
    Q_ASSERT_X(rc == 0, "Database::transaction", mdb_strerror(rc));

    if (!m_postingDB)
        m_postingDB = new PostingDB(m_txn);
    else
        m_postingDB->setTransaction(m_txn);

    if (!m_positionDB)
        m_positionDB = new PositionDB(m_txn);
    else
        m_positionDB->setTransaction(m_txn);

    if (!m_documentTermsDB)
        m_documentTermsDB = new DocumentDB(m_txn);
    else
        m_documentTermsDB->setTransaction(m_txn);

    if (!m_documentXattrTermsDB)
        m_documentXattrTermsDB = new DocumentDB(m_txn);
    else
        m_documentXattrTermsDB->setTransaction(m_txn);

    if (!m_documentFileNameTermsDB)
        m_documentFileNameTermsDB = new DocumentDB(m_txn);
    else
        m_documentFileNameTermsDB->setTransaction(m_txn);

    if (!m_docUrlDB)
        m_docUrlDB = new DocumentUrlDB(m_txn);
    else
        m_docUrlDB->setTransaction(m_txn);

    if (!m_docTimeDB)
        m_docTimeDB = new DocumentTimeDB(m_txn);
    else
        m_docTimeDB->setTransaction(m_txn);

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

bool Database::hasDocument(quint64 id)
{
    Q_ASSERT(id > 0);
    return m_documentTermsDB->contains(id);
}

quint64 Database::documentId(const QByteArray& url)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(!url.isEmpty());

    return filePathToId(url);
}

QByteArray Database::documentUrl(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    return m_docUrlDB->get(id);
}


quint64 Database::documentMTime(quint64 id)
{
    Q_ASSERT(m_txn);
    return m_docTimeDB->get(id).mTime;
}

quint64 Database::documentCTime(quint64 id)
{
    Q_ASSERT(m_txn);
    return m_docTimeDB->get(id).cTime;
}

QByteArray Database::documentData(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    return m_docDataDB->get(id);
}

void Database::setPhaseOne(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    m_contentIndexingDB->put(id);
}

void Database::addDocument(const Document& doc)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);

    quint64 id = doc.id();

    QVector<QByteArray> docTerms;
    docTerms.reserve(doc.m_terms.size());

    // FIXME: Add asserts to make sure the document does not already exist
    //        Otherwise we will have strange stale data
    QMapIterator<QByteArray, Document::TermData> it(doc.m_terms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    m_documentTermsDB->put(id, docTerms);

    QVector<QByteArray> docXattrTerms;
    docXattrTerms.reserve(doc.m_xattrTerms.size());

    it = QMapIterator<QByteArray, Document::TermData>(doc.m_xattrTerms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docXattrTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    if (!docXattrTerms.isEmpty())
        m_documentXattrTermsDB->put(id, docXattrTerms);

    QVector<QByteArray> docFileNameTerms;
    docFileNameTerms.reserve(doc.m_fileNameTerms.size());

    it = QMapIterator<QByteArray, Document::TermData>(doc.m_fileNameTerms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docFileNameTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    if (!docFileNameTerms.isEmpty())
        m_documentFileNameTermsDB->put(id, docFileNameTerms);

    if (!doc.url().isEmpty()) {
        m_docUrlDB->put(id, doc.url());
    }

    if (doc.contentIndexing()) {
        m_contentIndexingDB->put(doc.id());
    }

    DocumentTimeDB::TimeInfo info;
    info.mTime = doc.m_mTime;
    info.cTime = doc.m_cTime;
    info.julianDay = doc.m_julianDay;

    m_docTimeDB->put(id, info);

    if (!doc.m_data.isEmpty()) {
        m_docDataDB->put(id, doc.m_data);
    }
}

void Database::removeDocument(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    // FIXME: Optimize this. We do not need to combine them into one big vector
    QVector<QByteArray> terms = m_documentTermsDB->get(id) + m_documentXattrTermsDB->get(id) + m_documentFileNameTermsDB->get(id);
    if (terms.isEmpty()) {
        return;
    }

    for (const QByteArray& term : terms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }

    m_documentTermsDB->del(id);
    m_documentXattrTermsDB->del(id);
    m_documentFileNameTermsDB->del(id);

    m_docUrlDB->del(id);

    m_contentIndexingDB->del(id);
    m_docTimeDB->del(id);
    m_docDataDB->del(id);
}

void Database::replaceDocument(const Document& doc, const Database::DocumentOperations& operations)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);

    const quint64 id = doc.id();
    Q_ASSERT_X(hasDocument(id), "Database::replaceDocument", "Document does not exist");

    if (operations & DocumentTerms) {
        QVector<QByteArray> prevTerms = m_documentTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docTerms;
        docTerms.reserve(doc.m_terms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_terms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        m_documentTermsDB->put(id, docTerms);
    }

    if (operations & XAttrTerms) {
        QVector<QByteArray> prevTerms = m_documentXattrTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docXattrTerms;
        docXattrTerms.reserve(doc.m_xattrTerms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_xattrTerms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docXattrTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        if (!docXattrTerms.isEmpty())
            m_documentXattrTermsDB->put(id, docXattrTerms);
    }

    if (operations & FileNameTerms) {
        QVector<QByteArray> prevTerms = m_documentFileNameTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docFileNameTerms;
        docFileNameTerms.reserve(doc.m_fileNameTerms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_fileNameTerms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docFileNameTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        if (!docFileNameTerms.isEmpty())
            m_documentFileNameTermsDB->put(id, docFileNameTerms);
    }

    if (operations & DocumentUrl) {
        // FIXME: Replacing the documentUrl is actually quite complicated!
        Q_ASSERT(0);
        m_docUrlDB->put(id, doc.url());
    }

    // FIXME: What about contentIndexing?
    /*
    if (doc.contentIndexing()) {
        m_contentIndexingDB->put(doc.id());
    }
    */

    if (operations & DocumentTime) {
        DocumentTimeDB::TimeInfo info;
        info.mTime = doc.m_mTime;
        info.cTime = doc.m_cTime;
        info.julianDay = doc.m_julianDay;

        m_docTimeDB->put(id, info);
    }

    if (operations & DocumentData) {
        m_docDataDB->put(id, doc.m_data);
    }
}

template<typename T>
static void insert(QVector<T>& vec, const T& id)
{
    if (vec.isEmpty()) {
        vec.append(id);
    } else {
        auto it = std::upper_bound(vec.begin(), vec.end(), id);

        // Merge the id if it does not
        auto prev = it - 1;
        if (*prev != id) {
            vec.insert(it, id);
        }
    }
}

void Database::commit()
{
    Q_ASSERT(m_txn);

    qDebug() << "PendingOperations:" << m_pendingOperations.size();
    QHashIterator<QByteArray, QVector<Operation> > iter(m_pendingOperations);
    while (iter.hasNext()) {
        iter.next();

        const QByteArray& term = iter.key();
        const QVector<Operation> operations = iter.value();

        PostingList list = m_postingDB->get(term);
        QVector<PositionInfo> positionList;// = m_positionDB->get(term); // FIXME: We do not need to fetch this for all the terms

        for (const Operation& op : operations) {
            quint64 id = op.data.docId;

            if (op.type == AddId) {
                insert(list, id);

                if (!op.data.positions.isEmpty()) {
                    insert(positionList, op.data);
                }
            }
            else {
                list.removeOne(id);
                positionList.removeOne(PositionInfo(id));
            }
        }

        m_postingDB->put(term, list);
        if (!positionList.isEmpty()) {
            m_positionDB->put(term, positionList);
        }
    }

    m_pendingOperations.clear();

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

QVector<quint64> Database::fetchPhaseOneIds(int size)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(size > 0);
    return m_contentIndexingDB->fetchItems(size);
}

QList<QByteArray> Database::fetchTermsStartingWith(const QByteArray& term)
{
    Q_ASSERT(term.size() > 0);
    return m_postingDB->fetchTermsStartingWith(term);
}

uint Database::phaseOneSize()
{
    Q_ASSERT(m_txn);
    return m_contentIndexingDB->size();
}

uint Database::size()
{
    Q_ASSERT(m_txn);
    return m_documentTermsDB->size();
}


//
// Queries
//

PostingIterator* Database::toPostingIterator(const EngineQuery& query)
{
    if (query.leaf()) {
        if (query.op() == EngineQuery::Equal) {
            return m_postingDB->iter(query.term());
        } else if (query.op() == EngineQuery::StartsWith) {
            return m_postingDB->prefixIter(query.term());
        } else {
            Q_ASSERT(0);
            // FIXME: Implement position iterator
        }
    }

    Q_ASSERT(!query.subQueries().isEmpty());

    QVector<PostingIterator*> vec;
    vec.reserve(query.subQueries().size());

    if (query.op() == EngineQuery::Phrase) {
        for (const EngineQuery& q : query.subQueries()) {
            Q_ASSERT_X(q.leaf(), "Database::toPostingIterator", "Phrase queries must contain leaf queries");
            vec << m_positionDB->iter(q.term());
        }

        return new PhraseAndIterator(vec);
    }

    for (const EngineQuery& q : query.subQueries()) {
        vec << toPostingIterator(q);
    }

    if (query.op() == EngineQuery::And) {
        return new AndPostingIterator(vec);
    } else if (query.op() == EngineQuery::Or) {
        return new OrPostingIterator(vec);
    }

    Q_ASSERT(0);
    return 0;
}

QVector<quint64> Database::exec(const EngineQuery& query, int limit)
{
    Q_ASSERT(m_txn);

    QVector<quint64> results;
    PostingIterator* it = toPostingIterator(query);
    if (!it) {
        return results;
    }

    while (it->next() && limit) {
        results << it->docId();
        limit--;
    }

    return results;
}

//
// File path rename
//
void Database::renameFilePath(const QByteArray& origFilePath, const Document& doc)
{
    const QByteArray newFilePath = doc.url();

    // Get the id
    quint64 id = filePathToId(newFilePath);
    // FIXME: What about if the fileId changes because we have moved it to another device?
    Q_ASSERT(id);

    QByteArray oldFileName = origFilePath.mid(origFilePath.lastIndexOf('/'));
    QByteArray newFileName = newFilePath.mid(newFilePath.lastIndexOf('/'));

    // Update the id -> url db
    m_docUrlDB->rename(id, newFileName);

    replaceDocument(doc, FileNameTerms);
}

