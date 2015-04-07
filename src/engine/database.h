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

#ifndef BALOO_DATABASE_H
#define BALOO_DATABASE_H

#include "engine_export.h"
#include "document.h"
#include "databasedbis.h"

#include <QString>
#include <lmdb.h>

namespace Baloo {

class EngineQuery;
class PostingIterator;
class WriteTransaction;

class DatabaseTest;

class BALOO_ENGINE_EXPORT Database
{
public:
    Database(const QString& path);
    ~Database();

    QString path() const;
    bool open();

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

    void renameFilePath(quint64 id, const Document& newDoc);

    //
    // Transaction Handling
    //
    enum TransactionType {
        ReadOnly,
        ReadWrite
    };

    /**
     * Starts a transaction in which the database can be modified.
     * It is necessary to start a transaction in order to use any
     * of the Database functions.
     */
    void transaction(TransactionType type);
    void commit();
    void abort();

    bool hasChanges() const;

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

    QVector<quint64> fetchPhaseOneIds(int size);
    uint phaseOneSize();
    uint size();

    QList<QByteArray> fetchTermsStartingWith(const QByteArray& term);
private:
    QString m_path;

    MDB_env* m_env;
    MDB_txn* m_txn;

    DatabaseDbis m_dbis;
    WriteTransaction* m_writeTrans;

    friend class DatabaseTest;

    PostingIterator* toPostingIterator(const EngineQuery& query);
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::Database::DocumentOperations);

#endif // BALOO_DATABASE_H
