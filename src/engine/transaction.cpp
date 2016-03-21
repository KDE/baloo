/*
 * This file is part of the KDE Baloo project.
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

#include "transaction.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documentdatadb.h"
#include "mtimedb.h"

#include "document.h"
#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "writetransaction.h"
#include "idutils.h"
#include "database.h"
#include "databasesize.h"

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Transaction::Transaction(const Database& db, Transaction::TransactionType type)
    : m_dbis(db.m_dbis)
    , m_env(db.m_env)
    , m_writeTrans(0)
{
    uint flags = type == ReadOnly ? MDB_RDONLY : 0;
    int rc = mdb_txn_begin(db.m_env, NULL, flags, &m_txn);
    Q_ASSERT_X(rc == 0, "Transaction", mdb_strerror(rc));

    if (type == ReadWrite) {
        m_writeTrans = new WriteTransaction(m_dbis, m_txn);
    }
}

Transaction::Transaction(Database* db, Transaction::TransactionType type)
    : Transaction(*db, type)
{
}

Transaction::~Transaction()
{
    if (m_writeTrans)
        qWarning() << "Closing an active WriteTransaction without calling abort/commit";

    if (m_txn) {
        abort();
    }
}

bool Transaction::hasDocument(quint64 id) const
{
    Q_ASSERT(id > 0);

    IdFilenameDB idFilenameDb(m_dbis.idFilenameDbi, m_txn);
    return idFilenameDb.contains(id);
}

bool Transaction::inPhaseOne(quint64 id) const
{
    Q_ASSERT(id > 0);
    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.contains(id);
}

bool Transaction::hasFailed(quint64 id) const
{
    Q_ASSERT(id > 0);
    DocumentIdDB failedIdDb(m_dbis.failedIdDbi, m_txn);
    return failedIdDb.contains(id);
}

QByteArray Transaction::documentUrl(quint64 id) const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.get(id);
}

quint64 Transaction::documentId(const QByteArray& path) const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(!path.isEmpty());

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    QList<QByteArray> li = path.split('/');

    quint64 parentId = 0;
    for (const QByteArray& fileName : li) {
        if (fileName.isEmpty()) {
            continue;
        }

        parentId = docUrlDb.getId(parentId, fileName);
        if (!parentId) {
            return 0;
        }
    }

    return parentId;
}

DocumentTimeDB::TimeInfo Transaction::documentTimeInfo(quint64 id) const
{
    Q_ASSERT(m_txn);

    DocumentTimeDB docTimeDb(m_dbis.docTimeDbi, m_txn);
    return docTimeDb.get(id);
}

QByteArray Transaction::documentData(quint64 id) const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    DocumentDataDB docDataDb(m_dbis.docDataDbi, m_txn);
    return docDataDb.get(id);
}

bool Transaction::hasChanges() const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(m_writeTrans);
    return m_writeTrans->hasChanges();
}

QVector<quint64> Transaction::fetchPhaseOneIds(int size) const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(size > 0);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.fetchItems(size);
}

QVector<QByteArray> Transaction::fetchTermsStartingWith(const QByteArray& term) const
{
    Q_ASSERT(term.size() > 0);

    PostingDB postingDb(m_dbis.postingDbi, m_txn);
    return postingDb.fetchTermsStartingWith(term);
}

uint Transaction::phaseOneSize() const
{
    Q_ASSERT(m_txn);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.size();
}

uint Transaction::size() const
{
    Q_ASSERT(m_txn);

    DocumentDB docTermsDb(m_dbis.docTermsDbi, m_txn);
    return docTermsDb.size();
}

//
// Write Operations
//
void Transaction::setPhaseOne(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    contentIndexingDb.put(id);
}

void Transaction::removePhaseOne(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    contentIndexingDb.del(id);
}

void Transaction::addFailed(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    DocumentIdDB failedIdDb(m_dbis.failedIdDbi, m_txn);
    failedIdDb.put(id);
}

void Transaction::addDocument(const Document& doc)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->addDocument(doc);
}

void Transaction::removeDocument(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->removeDocument(id);
}

void Transaction::removeRecursively(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->removeRecursively(id);
}

void Transaction::replaceDocument(const Document& doc, DocumentOperations operations)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);
    Q_ASSERT(m_writeTrans);
    Q_ASSERT_X(hasDocument(doc.id()), "Transaction::replaceDocument", "Document does not exist");

    m_writeTrans->replaceDocument(doc, operations);
}

void Transaction::commit()
{
    Q_ASSERT(m_txn);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->commit();
    delete m_writeTrans;
    m_writeTrans = 0;

    int rc = mdb_txn_commit(m_txn);
    Q_ASSERT_X(rc == 0, "Transaction::commit", mdb_strerror(rc));

    m_txn = 0;
}

void Transaction::abort()
{
    Q_ASSERT(m_txn);

    mdb_txn_abort(m_txn);
    m_txn = 0;

    delete m_writeTrans;
    m_writeTrans = 0;
}

//
// Queries
//

PostingIterator* Transaction::postingIterator(const EngineQuery& query) const
{
    PostingDB postingDb(m_dbis.postingDbi, m_txn);
    PositionDB positionDb(m_dbis.positionDBi, m_txn);

    if (query.leaf()) {
        if (query.op() == EngineQuery::Equal) {
            return postingDb.iter(query.term());
        } else if (query.op() == EngineQuery::StartsWith) {
            return postingDb.prefixIter(query.term());
        } else {
            Q_ASSERT(0);
        }
    }

    if (query.subQueries().isEmpty()) {
        return 0;
    }

    QVector<PostingIterator*> vec;
    vec.reserve(query.subQueries().size());

    if (query.op() == EngineQuery::Phrase) {
        for (const EngineQuery& q : query.subQueries()) {
            Q_ASSERT_X(q.leaf(), "Transaction::toPostingIterator", "Phrase queries must contain leaf queries");
            vec << positionDb.iter(q.term());
        }

        return new PhraseAndIterator(vec);
    }

    for (const EngineQuery& q : query.subQueries()) {
        vec << postingIterator(q);
    }

    if (query.op() == EngineQuery::And) {
        return new AndPostingIterator(vec);
    } else if (query.op() == EngineQuery::Or) {
        return new OrPostingIterator(vec);
    }

    Q_ASSERT(0);
    return 0;
}

PostingIterator* Transaction::postingCompIterator(const QByteArray& prefix, const QByteArray& value, PostingDB::Comparator com) const
{
    PostingDB postingDb(m_dbis.postingDbi, m_txn);
    return postingDb.compIter(prefix, value, com);
}

PostingIterator* Transaction::mTimeIter(quint32 mtime, MTimeDB::Comparator com) const
{
    MTimeDB mTimeDb(m_dbis.mtimeDbi, m_txn);
    return mTimeDb.iter(mtime, com);
}

PostingIterator* Transaction::mTimeRangeIter(quint32 beginTime, quint32 endTime) const
{
    MTimeDB mTimeDb(m_dbis.mtimeDbi, m_txn);
    return mTimeDb.iterRange(beginTime, endTime);
}

PostingIterator* Transaction::docUrlIter(quint64 id) const
{
    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.iter(id);
}

QVector<quint64> Transaction::exec(const EngineQuery& query, int limit) const
{
    Q_ASSERT(m_txn);

    QVector<quint64> results;
    PostingIterator* it = postingIterator(query);
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
// Introspection
//

QVector<QByteArray> Transaction::documentTerms(quint64 docId) const
{
    Q_ASSERT(docId);

    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    return documentTermsDB.get(docId);
}

QVector<QByteArray> Transaction::documentFileNameTerms(quint64 docId) const
{
    Q_ASSERT(docId);

    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    return documentFileNameTermsDB.get(docId);
}

QVector<QByteArray> Transaction::documentXattrTerms(quint64 docId) const
{
    Q_ASSERT(docId);

    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    return documentXattrTermsDB.get(docId);
}

//
// File Size
//
static uint dbiSize(MDB_txn* txn, MDB_dbi dbi)
{
    MDB_stat stat;
    mdb_stat(txn, dbi, &stat);

    return (stat.ms_branch_pages + stat.ms_leaf_pages + stat.ms_overflow_pages) * stat.ms_psize;
}

DatabaseSize Transaction::dbSize()
{
    DatabaseSize dbSize;
    dbSize.postingDb = dbiSize(m_txn, m_dbis.postingDbi);
    dbSize.positionDb = dbiSize(m_txn, m_dbis.positionDBi);
    dbSize.docTerms = dbiSize(m_txn, m_dbis.docTermsDbi);
    dbSize.docFilenameTerms = dbiSize(m_txn, m_dbis.docFilenameTermsDbi);
    dbSize.docXattrTerms = dbiSize(m_txn, m_dbis.docXattrTermsDbi);

    dbSize.idTree = dbiSize(m_txn, m_dbis.idTreeDbi);
    dbSize.idFilename = dbiSize(m_txn, m_dbis.idFilenameDbi);

    dbSize.docTime = dbiSize(m_txn, m_dbis.docTimeDbi);
    dbSize.docData = dbiSize(m_txn, m_dbis.docDataDbi);

    dbSize.contentIndexingIds = dbiSize(m_txn, m_dbis.contentIndexingDbi);
    dbSize.failedIds = dbiSize(m_txn, m_dbis.failedIdDbi);

    dbSize.mtimeDb = dbiSize(m_txn, m_dbis.mtimeDbi);

    dbSize.expectedSize = dbSize.positionDb + dbSize.positionDb + dbSize.docTerms + dbSize.docFilenameTerms
                  + dbSize.docXattrTerms + dbSize.idTree + dbSize.idFilename + dbSize.docTime
                  + dbSize.docData + dbSize.contentIndexingIds + dbSize.failedIds + dbSize.mtimeDb;

    MDB_envinfo info;
    mdb_env_info(m_env, &info);
    dbSize.actualSize = info.me_last_pgno * 4096; // TODO: separate page size

    return dbSize;
}

//
// Debugging
//
void Transaction::checkFsTree()
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    PostingDB postingDb(m_dbis.postingDbi, m_txn);

    auto map = postingDb.toTestMap();

    QList<PostingList> allLists = map.values();
    QSet<quint64> allIds;
    for (auto list : allLists) {
        for (quint64 id : list) {
            allIds << id;
        }
    }

    QTextStream out(stdout);
    out << "Total Document IDs: " << allIds.size() << endl;

    int count = 0;
    for (quint64 id: allIds) {
        QByteArray url = docUrlDb.get(id);
        if (url.isEmpty()) {
            auto terms = documentTermsDB.get(id);
            auto fileNameTerms = documentFileNameTermsDB.get(id);
            auto xAttrTerms = documentXattrTermsDB.get(id);

            // Lets reverse enginer the terms
            QList<QByteArray> newTerms;
            QMapIterator<QByteArray, PostingList> it(map);
            while (it.hasNext()) {
                it.next();
                if (it.value().contains(id)) {
                    newTerms << it.key();
                }
            }

            out << "Missing filePath for " << id << endl;
            out << "\tPostingDB Terms: ";
            for (const QByteArray& term : newTerms) {
                out << term << " ";
            }
            out << endl;

            out << "\tDocumentTermsDB: ";
            for (const QByteArray& term : terms) {
                out << term << " ";
            }
            out << endl;

            out << "\tFileNameTermsDB: ";
            for (const QByteArray& term : terms) {
                out << term << " ";
            }
            out << endl;

            out << "\tXAttrTermsDB: ";
            for (const QByteArray& term : terms) {
                out << term << " ";
            }
            out << endl;

            count++;
        }
    }

    out << "Invalid Entries: " << count << " (" << count * 100.0 / allIds.size() << "%)" << endl;
}

void Transaction::checkTermsDbinPostingDb()
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    PostingDB postingDb(m_dbis.postingDbi, m_txn);

    // Iterate over each document, and fetch all terms
    // check if each term maps to its own id in the posting db

    auto map = postingDb.toTestMap();

    QList<PostingList> allLists = map.values();
    QSet<quint64> allIds;
    for (auto list : allLists) {
        for (quint64 id : list) {
            allIds << id;
        }
    }

    QTextStream out(stdout);
    out << "PostingDB check .." << endl;
    for (quint64 id : allIds) {
        QVector<QByteArray> terms = documentTermsDB.get(id);
        terms += documentXattrTermsDB.get(id);
        terms += documentFileNameTermsDB.get(id);

        for (const QByteArray& term : terms) {
            PostingList plist = postingDb.get(term);
            if (!plist.contains(id)) {
                out << id << " is missing term " << term << endl;
            }
        }
    }
}

void Transaction::checkPostingDbinTermsDb()
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    PostingDB postingDb(m_dbis.postingDbi, m_txn);

    QMap<QByteArray, PostingList> map = postingDb.toTestMap();
    QMapIterator<QByteArray, PostingList> it(map);

    QTextStream out(stdout);
    out << "DocumentTermsDB check .." << endl;
    while (it.hasNext()) {
        it.next();

        const QByteArray term = it.key();
        const PostingList list = it.value();
        for (quint64 id : list) {
            if (documentTermsDB.get(id).contains(term)) {
                continue;
            }
            if (documentFileNameTermsDB.get(id).contains(term)) {
                continue;
            }
            if (documentXattrTermsDB.get(id).contains(term)) {
                continue;
            }
            out << id << " is missing " << QString::fromUtf8(term) << " from document terms db" << endl;
        }
    }
}

