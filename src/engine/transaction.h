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
#include "engine_export.h"

#include <QString>
#include <lmdb.h>

namespace Baloo {

class Database;
class Document;
class PostingIterator;
class WriteTransaction;
class EngineQuery;
class DatabaseTest;

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
    bool hasDocument(quint64 id);
    QByteArray documentUrl(quint64 id);
    quint64 documentId(quint64 parentId, const QByteArray& fileName);
    QByteArray documentData(quint64 id);

    quint64 documentMTime(quint64 id);
    quint64 documentCTime(quint64 id);

    QVector<quint64> exec(const EngineQuery& query, int limit = -1);

    PostingIterator* postingIterator(const EngineQuery& query);
    PostingIterator* postingCompIterator(const QByteArray& prefix, const QByteArray& value, PostingDB::Comparator com);
    PostingIterator* mTimeIter(quint32 mtime, MTimeDB::Comparator com);
    PostingIterator* mTimeRangeIter(quint32 beginTime, quint32 endTime);
    PostingIterator* docUrlIter(quint64 id);

    QVector<quint64> fetchPhaseOneIds(int size);
    uint phaseOneSize();
    uint size();

    QVector<QByteArray> fetchTermsStartingWith(const QByteArray& term);

    //
    // Introspecing document data
    //
    QVector<QByteArray> documentTerms(quint64 docId);
    QVector<QByteArray> documentFileNameTerms(quint64 docId);
    QVector<QByteArray> documentXattrTerms(quint64 docId);

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

    enum DocumentOperation {
        DocumentTerms =  1,
        FileNameTerms =  2,
        XAttrTerms    =  4,
        DocumentData  =  8,
        DocumentUrl   = 16,
        DocumentTime  = 32,
        Everything    = DocumentTerms | FileNameTerms | XAttrTerms | DocumentData | DocumentUrl | DocumentTime
    };
    Q_DECLARE_FLAGS(DocumentOperations, DocumentOperation)

    void replaceDocument(const Document& doc, DocumentOperations operations);
    void setPhaseOne(quint64 id);
    void removePhaseOne(quint64 id);

    void renameFilePath(quint64 id, const Document& newDoc);

private:
    Transaction(const Transaction& rhs);

    const DatabaseDbis& m_dbis;
    MDB_txn* m_txn;
    WriteTransaction* m_writeTrans;

    friend class DatabaseTest;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::Transaction::DocumentOperations);

#endif // BALOO_TRANSACTION_H
