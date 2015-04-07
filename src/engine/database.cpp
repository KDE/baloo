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
#include "mtimedb.h"

#include "document.h"
#include "enginequery.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "phraseanditerator.h"

#include "writetransaction.h"
#include "idutils.h"

#include <QFile>
#include <QFileInfo>

using namespace Baloo;

Database::Database(const QString& path)
    : m_path(path)
    , m_env(0)
    , m_txn(0)
    , m_writeTrans(0)
{
}

Database::~Database()
{
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
    mdb_env_set_maxdbs(m_env, 11);
    mdb_env_set_mapsize(m_env, 1048576000);

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(m_path) + "/index";
    mdb_env_open(m_env, arr.constData(), MDB_NOSUBDIR, 0664);

    return true;
}

void Database::transaction(Database::TransactionType type)
{
    Q_ASSERT(m_txn == 0);

    uint flags = type == ReadOnly ? MDB_RDONLY : 0;
    int rc = mdb_txn_begin(m_env, NULL, flags, &m_txn);
    Q_ASSERT_X(rc == 0, "Database::transaction", mdb_strerror(rc));

    // First time
    if (m_dbis.postingDbi == 0) {
        m_dbis.postingDbi = PostingDB::create(m_txn);
        m_dbis.positionDBi = PositionDB::create(m_txn);

        // FIXME: All of these are the same
        m_dbis.docTermsDbi = DocumentDB::create("docterms", m_txn);
        m_dbis.docFilenameTermsDbi = DocumentDB::create("docfilenameterms", m_txn);
        m_dbis.docXattrTermsDbi = DocumentDB::create("docxatrrterms", m_txn);

        m_dbis.idTreeDbi = IdTreeDB::create(m_txn);
        m_dbis.idFilenameDbi = IdFilenameDB::create(m_txn);

        m_dbis.docTimeDbi = DocumentTimeDB::create(m_txn);
        m_dbis.docDataDbi = DocumentDataDB::create(m_txn);
        m_dbis.contentIndexingDbi = DocumentIdDB::create(m_txn);

        m_dbis.mtimeDbi = MTimeDB::create(m_txn);
    }

    if (type == ReadWrite) {
        Q_ASSERT(m_writeTrans == 0);
        m_writeTrans = new WriteTransaction(m_dbis, m_txn);
    }
}

QString Database::path() const
{
    return m_path;
}

bool Database::hasDocument(quint64 id)
{
    Q_ASSERT(id > 0);

    DocumentDB docTermsDB(m_dbis.docTermsDbi, m_txn);
    return docTermsDB.contains(id);
}

QByteArray Database::documentUrl(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.get(id);
}

quint64 Database::documentId(quint64 parentId, const QByteArray& fileName)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(parentId > 0);
    Q_ASSERT(!fileName.isEmpty());

    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    return docUrlDb.getId(parentId, fileName);
}

quint64 Database::documentMTime(quint64 id)
{
    Q_ASSERT(m_txn);

    DocumentTimeDB docTimeDb(m_dbis.docTermsDbi, m_txn);
    return docTimeDb.get(id).mTime;
}

quint64 Database::documentCTime(quint64 id)
{
    Q_ASSERT(m_txn);

    DocumentTimeDB docTimeDb(m_dbis.docTermsDbi, m_txn);
    return docTimeDb.get(id).cTime;
}

QByteArray Database::documentData(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);

    DocumentDataDB docDataDb(m_dbis.docDataDbi, m_txn);
    return docDataDb.get(id);
}

bool Database::hasChanges() const
{
    Q_ASSERT(m_txn);
    Q_ASSERT(m_writeTrans);
    return m_writeTrans->hasChanges();
}

QVector<quint64> Database::fetchPhaseOneIds(int size)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(size > 0);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.fetchItems(size);
}

QList<QByteArray> Database::fetchTermsStartingWith(const QByteArray& term)
{
    Q_ASSERT(term.size() > 0);

    PostingDB postingDb(m_dbis.postingDbi, m_txn);
    return postingDb.fetchTermsStartingWith(term);
}

uint Database::phaseOneSize()
{
    Q_ASSERT(m_txn);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    return contentIndexingDb.size();
}

uint Database::size()
{
    Q_ASSERT(m_txn);

    DocumentDB docTermsDb(m_dbis.docTermsDbi, m_txn);
    return docTermsDb.size();
}

//
// Write Operations
//
void Database::setPhaseOne(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    DocumentIdDB contentIndexingDb(m_dbis.contentIndexingDbi, m_txn);
    contentIndexingDb.put(id);
}

void Database::addDocument(const Document& doc)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->addDocument(doc);
}

void Database::removeDocument(quint64 id)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(id > 0);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->removeDocument(id);
}

void Database::replaceDocument(const Document& doc, Database::DocumentOperations operations)
{
    Q_ASSERT(m_txn);
    Q_ASSERT(doc.id() > 0);
    Q_ASSERT(m_writeTrans);
    Q_ASSERT_X(hasDocument(doc.id()), "Database::replaceDocument", "Document does not exist");

    m_writeTrans->replaceDocument(doc, operations);
}

void Database::commit()
{
    Q_ASSERT(m_txn);
    Q_ASSERT(m_writeTrans);

    m_writeTrans->commit();
    delete m_writeTrans;
    m_writeTrans = 0;

    int rc = mdb_txn_commit(m_txn);
    Q_ASSERT_X(rc == 0, "Database::commit", mdb_strerror(rc));

    m_txn = 0;
}

void Database::abort()
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

PostingIterator* Database::toPostingIterator(const EngineQuery& query)
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
            // FIXME: Implement position iterator
        }
    }

    Q_ASSERT(!query.subQueries().isEmpty());

    QVector<PostingIterator*> vec;
    vec.reserve(query.subQueries().size());

    if (query.op() == EngineQuery::Phrase) {
        for (const EngineQuery& q : query.subQueries()) {
            Q_ASSERT_X(q.leaf(), "Database::toPostingIterator", "Phrase queries must contain leaf queries");
            vec << positionDb.iter(q.term());
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
void Database::renameFilePath(quint64 id, const Document& newDoc)
{
    Q_ASSERT(id);

    const QByteArray newFilePath = newDoc.url();
    const QByteArray newFileName = newFilePath.mid(newFilePath.lastIndexOf('/') + 1);

    // Update the id -> url db
    DocumentUrlDB docUrlDb(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);
    docUrlDb.rename(id, newFileName);

    replaceDocument(newDoc, FileNameTerms);
}

