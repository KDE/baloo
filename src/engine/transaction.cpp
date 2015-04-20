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
#include "documenttimedb.h"
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

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Transaction::Transaction(const Database& db, Transaction::TransactionType type)
    : m_dbis(db.m_dbis)
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

Transaction::Transaction(const Transaction& rhs)
    : m_dbis(rhs.m_dbis)
{
    Q_ASSERT(0);
}

Transaction::~Transaction()
{
    if (m_writeTrans)
        qWarning() << "Closing an active WriteTransaction without calling abort/commit";

    if (m_txn) {
        abort();
    }
}

bool Transaction::hasDocument(quint64 id)
{
    Q_ASSERT(id > 0);

    IdFilenameDB idFilenameDb(m_dbis.idFilenameDbi, m_txn);
    return idFilenameDb.contains(id);
}

QByteArray Transaction::documentUrl(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.get(id);
}

quint64 Transaction::documentId(quint64 parentId, const QByteArray& fileName)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(parentId > 0);
    Q_ASSERT(!fileName.isEmpty());

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.getId(parentId, fileName);
}

quint64 Transaction::documentMTime(quint64 id)
{
    Q_ASSERT(m_txn);

    DocumentTimeDB docTimeDb(m_dbis.docTermsDbi, m_txn);
    return docTimeDb.get(id).mTime;
}

quint64 Transaction::documentCTime(quint64 id)
{
    Q_ASSERT(m_txn);

    DocumentTimeDB docTimeDb(m_dbis.docTermsDbi, m_txn);
    return docTimeDb.get(id).cTime;
}

QByteArray Transaction::documentData(quint64 id)
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

QVector<quint64> Transaction::fetchPhaseOneIds(int size)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(size > 0);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.fetchItems(size);
}

QVector< QByteArray > Transaction::fetchTermsStartingWith(const QByteArray& term)
{
    Q_ASSERT(term.size() > 0);

    PostingDB postingDb(m_dbis.postingDbi, m_txn);
    return postingDb.fetchTermsStartingWith(term);
}

uint Transaction::phaseOneSize()
{
    Q_ASSERT(m_txn);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.size();
}

uint Transaction::size()
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

void Transaction::replaceDocument(const Document& doc, Transaction::DocumentOperations operations)
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

PostingIterator* Transaction::postingIterator(const EngineQuery& query)
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

    Q_ASSERT(!query.subQueries().isEmpty());

    QVector<PostingIterator*> vec;
    vec.reserve(query.subQueries().size());

    if (query.op() == EngineQuery::Phrase) {
        for (const EngineQuery& q : query.subQueries()) {
            Q_ASSERT_X(q.leaf(), "Transaction::toPostingIterator", "Phrase queries must contain leaf queries");
            auto it = positionDb.iter(q.term());
            if (it) {
                vec << it;
            }
        }

        return new PhraseAndIterator(vec);
    }

    for (const EngineQuery& q : query.subQueries()) {
        auto it = postingIterator(q);
        if (it) {
            vec << it;
        }
    }

    if (query.op() == EngineQuery::And) {
        return new AndPostingIterator(vec);
    } else if (query.op() == EngineQuery::Or) {
        return new OrPostingIterator(vec);
    }

    Q_ASSERT(0);
    return 0;
}

PostingIterator* Transaction::mTimeIter(quint32 mtime, MTimeDB::Comparator com)
{
    MTimeDB mTimeDb(m_dbis.mtimeDbi, m_txn);
    return mTimeDb.iter(mtime, com);
}

PostingIterator* Transaction::mTimeRangeIter(quint32 beginTime, quint32 endTime)
{
    MTimeDB mTimeDb(m_dbis.mtimeDbi, m_txn);
    return mTimeDb.iterRange(beginTime, endTime);
}

PostingIterator* Transaction::docUrlIter(quint64 id)
{
    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.iter(id);
}

QVector<quint64> Transaction::exec(const EngineQuery& query, int limit)
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
// File path rename
//
void Transaction::renameFilePath(quint64 id, const Document& newDoc)
{
    Q_ASSERT(id);

    // Update the id -> url db
    // TODO: Use something more efficient than a del + put
    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    docUrlDb.del(id);
    docUrlDb.put(id, newDoc.url());

    replaceDocument(newDoc, FileNameTerms);
}
