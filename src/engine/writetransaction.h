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

#ifndef BALOO_WRITETRANSACTION_H
#define BALOO_WRITETRANSACTION_H

#include "engine_export.h"
#include "database.h"
#include "positiondb.h" // reqd for PositionInfo

namespace Baloo {

class PostingDB;
class PositionDB;
class DocumentDB;
class DocumentDataDB;
class DocumentUrlDB;
class DocumentTimeDB;
class DocumentIdDB;
class MTimeDB;

class WriteTransaction
{
public:
    WriteTransaction(PostingDB* postingDB, PositionDB* positionDB, DocumentDB* docTerms,
                     DocumentDB* docXattrTerms, DocumentDB* docFileNameTerms, DocumentUrlDB* docUrlDB,
                     DocumentTimeDB* docTimeDB, DocumentDataDB* docDataDB, DocumentIdDB* contentIndexingDB,
                     MTimeDB* mtimeDB);

    void addDocument(const Document& doc);
    void removeDocument(quint64 id);
    void replaceDocument(const Document& doc, Database::DocumentOperations operations);
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
    QHash<QByteArray, QVector<Operation> > m_pendingOperations;

    PostingDB* m_postingDB;
    PositionDB* m_positionDB;

    DocumentDB* m_documentTermsDB;
    DocumentDB* m_documentXattrTermsDB;
    DocumentDB* m_documentFileNameTermsDB;

    DocumentUrlDB* m_docUrlDB;

    DocumentTimeDB* m_docTimeDB;
    DocumentDataDB* m_docDataDB;
    DocumentIdDB* m_contentIndexingDB;
    MTimeDB* m_mtimeDB;
};
}

Q_DECLARE_TYPEINFO(Baloo::WriteTransaction::Operation, Q_MOVABLE_TYPE);

#endif // BALOO_WRITETRANSACTION_H
