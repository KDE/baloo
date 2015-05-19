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

#ifndef BALOO_WRITETRANSACTION_H
#define BALOO_WRITETRANSACTION_H

#include "engine_export.h"
#include "transaction.h"
#include "positioninfo.h"
#include "document.h"

namespace Baloo {

class WriteTransaction
{
public:
    WriteTransaction(DatabaseDbis dbis, MDB_txn* txn)
        : m_txn(txn)
        , m_dbis(dbis)
    {}

    void addDocument(const Document& doc);
    void removeDocument(quint64 id);

    /**
     * Remove the document with id \p parentId and all its children.
     */
    void removeRecursively(quint64 parentId);
    void replaceDocument(const Document& doc, Transaction::DocumentOperations operations);
    void commit();

    bool hasChanges() const {
        return !m_pendingOperations.isEmpty();
    }
    enum OperationType {
        AddId,
        RemoveId
    };
    struct Operation {
        OperationType type;
        PositionInfo data;
    };

private:
    /*
     * Adds an 'addId' operation to the pending queue for each term.
     * Returns the list of all the terms.
     */
    QVector<QByteArray> addTerms(quint64 id, const QMap<QByteArray, Document::TermData>& terms);
    QVector<QByteArray> replaceTerms(quint64 id, const QVector<QByteArray>& prevTerms,
                                     const QMap<QByteArray, Document::TermData>& terms);
    void removeTerms(quint64 id, const QVector<QByteArray>& terms);

    QHash<QByteArray, QVector<Operation> > m_pendingOperations;

    MDB_txn* m_txn;
    DatabaseDbis m_dbis;
};
}

Q_DECLARE_TYPEINFO(Baloo::WriteTransaction::Operation, Q_MOVABLE_TYPE);

#endif // BALOO_WRITETRANSACTION_H
