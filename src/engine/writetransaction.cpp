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

#include "writetransaction.h"
#include "transaction.h"

#include "document.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"
#include "mtimedb.h"

using namespace Baloo;

void WriteTransaction::addDocument(const Document& doc)
{
    quint64 id = doc.id();

    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    Q_ASSERT(!documentTermsDB.contains(id));
    Q_ASSERT(!documentXattrTermsDB.contains(id));
    Q_ASSERT(!documentFileNameTermsDB.contains(id));
    Q_ASSERT(!docTimeDB.contains(id));
    Q_ASSERT(!docDataDB.contains(id));
    Q_ASSERT(!contentIndexingDB.contains(id));

    if (!docUrlDB.put(id, doc.url())) {
        return;
    }

    QVector<QByteArray> docTerms = addTerms(id, doc.m_terms);
    documentTermsDB.put(id, docTerms);

    QVector<QByteArray> docXattrTerms = addTerms(id, doc.m_xattrTerms);
    if (!docXattrTerms.isEmpty())
        documentXattrTermsDB.put(id, docXattrTerms);

    QVector<QByteArray> docFileNameTerms = addTerms(id, doc.m_fileNameTerms);
    if (!docFileNameTerms.isEmpty())
        documentFileNameTermsDB.put(id, docFileNameTerms);


    if (doc.contentIndexing()) {
        contentIndexingDB.put(doc.id());
    }

    DocumentTimeDB::TimeInfo info;
    info.mTime = doc.m_mTime;
    info.cTime = doc.m_cTime;

    docTimeDB.put(id, info);
    mtimeDB.put(doc.m_mTime, id);

    if (!doc.m_data.isEmpty()) {
        docDataDB.put(id, doc.m_data);
    }
}

QVector<QByteArray> WriteTransaction::addTerms(quint64 id, const QMap<QByteArray, Document::TermData>& terms)
{
    QVector<QByteArray> termList;
    termList.reserve(terms.size());

    QMapIterator<QByteArray, Document::TermData> it(terms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        termList.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    return termList;
}


void WriteTransaction::removeDocument(quint64 id)
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    DocumentIdDB failedIndexingDB(m_dbis.failedIdDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    removeTerms(id, documentTermsDB.get(id));
    removeTerms(id, documentXattrTermsDB.get(id));
    removeTerms(id, documentFileNameTermsDB.get(id));

    documentTermsDB.del(id);
    documentXattrTermsDB.del(id);
    documentFileNameTermsDB.del(id);

    docUrlDB.del(id, [&docTimeDB](quint64 id) {
        return !docTimeDB.contains(id);
    });

    contentIndexingDB.del(id);
    failedIndexingDB.del(id);

    DocumentTimeDB::TimeInfo info = docTimeDB.get(id);
    if (info.mTime) {
        docTimeDB.del(id);
        mtimeDB.del(info.mTime, id);
    }

    docDataDB.del(id);
}

void WriteTransaction::removeTerms(quint64 id, const QVector<QByteArray>& terms)
{
    for (const QByteArray& term : terms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }
}

void WriteTransaction::removeRecursively(quint64 parentId)
{
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    const QVector<quint64> children = docUrlDB.getChildren(parentId);
    for (quint64 id : children) {
        removeRecursively(id);
    }
    removeDocument(parentId);
}

void WriteTransaction::replaceDocument(const Document& doc, DocumentOperations operations)
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    const quint64 id = doc.id();

    if (operations & DocumentTerms) {
        Q_ASSERT(!doc.m_terms.isEmpty());
        QVector<QByteArray> prevTerms = documentTermsDB.get(id);
        QVector<QByteArray> docTerms = replaceTerms(id, prevTerms, doc.m_terms);

        documentTermsDB.put(id, docTerms);
    }

    if (operations & XAttrTerms) {
        QVector<QByteArray> prevTerms = documentXattrTermsDB.get(id);
        QVector<QByteArray> docXattrTerms = replaceTerms(id, prevTerms, doc.m_xattrTerms);

        if (!docXattrTerms.isEmpty())
            documentXattrTermsDB.put(id, docXattrTerms);
        else
            documentXattrTermsDB.del(id);
    }

    if (operations & FileNameTerms) {
        QVector<QByteArray> prevTerms = documentFileNameTermsDB.get(id);
        QVector<QByteArray> docFileNameTerms = replaceTerms(id, prevTerms, doc.m_fileNameTerms);

        if (!docFileNameTerms.isEmpty())
            documentFileNameTermsDB.put(id, docFileNameTerms);
        else
            documentFileNameTermsDB.del(id);
    }

    if (operations & DocumentTime) {
        Q_ASSERT(doc.m_mTime);
        Q_ASSERT(doc.m_cTime);

        DocumentTimeDB::TimeInfo info;
        info.mTime = doc.m_mTime;
        info.cTime = doc.m_cTime;

        docTimeDB.put(id, info);
        mtimeDB.put(doc.m_mTime, id);
    }

    if (operations & DocumentData) {
        if (!doc.m_data.isEmpty()) {
            docDataDB.put(id, doc.m_data);
        } else {
            docDataDB.del(id);
        }
    }

    if (operations & DocumentUrl) {
        docUrlDB.replace(id, doc.url(), [&docTimeDB](quint64 id) {
            return !docTimeDB.contains(id);
        });;
    }
}

QVector< QByteArray > WriteTransaction::replaceTerms(quint64 id, const QVector<QByteArray>& prevTerms,
                                                     const QMap<QByteArray, Document::TermData>& terms)
{
    for (const QByteArray& term : prevTerms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }

    return addTerms(id, terms);
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

template<typename T>
static void removeOne(QVector<T>& vec, const T& id)
{
    const int idx = vec.indexOf(id);
    if (idx >= 0) {
        vec.remove(idx);
    }
}

void WriteTransaction::commit()
{
    PostingDB postingDB(m_dbis.postingDbi, m_txn);
    PositionDB positionDB(m_dbis.positionDBi, m_txn);

    QHashIterator<QByteArray, QVector<Operation> > iter(m_pendingOperations);
    while (iter.hasNext()) {
        iter.next();

        const QByteArray& term = iter.key();
        const QVector<Operation> operations = iter.value();

        PostingList list = postingDB.get(term);

        bool fetchedPositionList = false;
        QVector<PositionInfo> positionList;

        for (const Operation& op : operations) {
            quint64 id = op.data.docId;

            if (op.type == AddId) {
                insert(list, id);

                if (!op.data.positions.isEmpty()) {
                    if (!fetchedPositionList) {
                        positionList = positionDB.get(term);
                        fetchedPositionList = true;
                    }
                    insert(positionList, op.data);
                }
            }
            else {
                removeOne(list, id);
                if (!fetchedPositionList) {
                    positionList = positionDB.get(term);
                    fetchedPositionList = true;
                }
                removeOne(positionList, PositionInfo(id));
            }
        }

        if (!list.isEmpty()) {
            postingDB.put(term, list);
        } else {
            postingDB.del(term);
        }

        if (fetchedPositionList) {
            if (!positionList.isEmpty()) {
                positionDB.put(term, positionList);
            } else {
                positionDB.del(term);
            }
        }
    }

    m_pendingOperations.clear();
}
