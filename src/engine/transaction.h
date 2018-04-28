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

#ifndef BALOO_TRANSACTION_H
#define BALOO_TRANSACTION_H

#include "databasedbis.h"
#include "mtimedb.h"
#include "postingdb.h"
#include "writetransaction.h"
#include "documenttimedb.h"

#include <QString>
#include <lmdb.h>

namespace Baloo {

class Database;
class Document;
class PostingIterator;
class EngineQuery;
class DatabaseSize;
class DBState;

class BALOO_ENGINE_EXPORT Transaction
{
public:
    enum TransactionType {
        ReadOnly,
        ReadWrite
    };
    Transaction(const Database& db, TransactionType type);
    Transaction(Database* db, TransactionType type);
    ~Transaction();

    //
    // Getters
    //
    bool hasDocument(quint64 id) const;
    bool inPhaseOne(quint64 id) const;
    bool hasFailed(quint64 id) const;
    QByteArray documentUrl(quint64 id) const;

    /**
     * This method is not cheap, and does not stat the filesystem in order to convert the path
     * \p path into an id.
     */
    quint64 documentId(const QByteArray& path) const;
    QVector<quint64> childrenDocumentId(quint64 parentId) const;
    QByteArray documentData(quint64 id) const;

    DocumentTimeDB::TimeInfo documentTimeInfo(quint64 id) const;

    QVector<quint64> exec(const EngineQuery& query, int limit = -1) const;

    PostingIterator* postingIterator(const EngineQuery& query) const;
    PostingIterator* postingCompIterator(const QByteArray& prefix, const QByteArray& value, PostingDB::Comparator com) const;
    PostingIterator* mTimeIter(quint32 mtime, MTimeDB::Comparator com) const;
    PostingIterator* mTimeRangeIter(quint32 beginTime, quint32 endTime) const;
    PostingIterator* docUrlIter(quint64 id) const;

    QVector<quint64> fetchPhaseOneIds(int size) const;
    uint phaseOneSize() const;
    uint size() const;

    QVector<QByteArray> fetchTermsStartingWith(const QByteArray& term) const;

    //
    // Introspecing document data
    //
    QVector<QByteArray> documentTerms(quint64 docId) const;
    QVector<QByteArray> documentFileNameTerms(quint64 docId) const;
    QVector<QByteArray> documentXattrTerms(quint64 docId) const;

    DatabaseSize dbSize();

    //
    // Transaction handling
    //
    void commit();
    void abort();
    bool hasChanges() const;

    //
    // Write Methods
    //
    void addDocument(const Document& doc);
    void removeDocument(quint64 id);
    void removeRecursively(quint64 parentId);
    void addFailed(quint64 id);

    template <typename Functor>
    void removeRecursively(quint64 id, Functor shouldDelete) {
        Q_ASSERT(m_txn);
        Q_ASSERT(m_writeTrans);

        m_writeTrans->removeRecursively(id, shouldDelete);
    }

    void replaceDocument(const Document& doc, DocumentOperations operations);
    void setPhaseOne(quint64 id);
    void removePhaseOne(quint64 id);

    // Debugging
    void checkFsTree();
    void checkTermsDbinPostingDb();
    void checkPostingDbinTermsDb();

private:
    Transaction(const Transaction& rhs) = delete;

    const DatabaseDbis& m_dbis;
    MDB_txn* m_txn;
    MDB_env* m_env;
    WriteTransaction* m_writeTrans;

    friend class DatabaseSanitizerImpl;
    friend class DBState; // for testing
};
}

#endif // BALOO_TRANSACTION_H
