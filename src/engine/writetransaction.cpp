/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "writetransaction.h"
#include "transaction.h"

#include "postingdb.h"
#include "documentdb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"
#include "mtimedb.h"
#include "idutils.h"

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

    static constexpr size_t fixedTermSize = 16; // fixed size guess of what length a term might have. not based on anything
    static constexpr auto fixedFileNameSize = static_cast<size_t>(NAME_MAX / 2 /* assume a half bad case */);
    static constexpr auto sizeOfId = sizeof(id);

    if (!docUrlDB.put(id, doc.url())) {
        return;
    }
    m_approximatelyPendingData += doc.url().size();
    // Internally DocumentUrlDB also updates the list of sub documents inside the parent dir. This is a bit tricky to approximate
    // without also adding size tracking to DocumentUrlDB (which is undesirable compared to keeping all the code in a single class)
    // because we don't know how many sub documents are our siblings, and it's hard to say how this exactly will apply dirt
    // to pages on the OS level. To get a somewhat flexible guess we are tracking the documents added as part of our
    // transaction and then assume each of them causes a dirty id and that this causes progressively worse memory use
    // (i.e. that is why we multiplicatively accumulate footprint of documents).
    // It also inserts the name portion of the path, that is easy enough to approximate.
    m_addedDocuments += 1;
    m_approximatelyPendingData += sizeOfId * m_addedDocuments + fixedFileNameSize;

    QVector<QByteArray> docTerms = addTerms(id, doc.m_terms);
    documentTermsDB.put(id, docTerms);
    m_approximatelyPendingData += docTerms.size() * fixedTermSize;

    QVector<QByteArray> docXattrTerms = addTerms(id, doc.m_xattrTerms);
    if (!docXattrTerms.isEmpty()) {
        documentXattrTermsDB.put(id, docXattrTerms);
        m_approximatelyPendingData += docXattrTerms.size() * fixedTermSize;
    }

    QVector<QByteArray> docFileNameTerms = addTerms(id, doc.m_fileNameTerms);
    if (!docFileNameTerms.isEmpty()) {
        documentFileNameTermsDB.put(id, docFileNameTerms);
        m_approximatelyPendingData += docFileNameTerms.size() * fixedFileNameSize;
    }

    if (doc.contentIndexing()) {
        contentIndexingDB.put(doc.id());
        m_approximatelyPendingData += sizeOfId;
    }

    DocumentTimeDB::TimeInfo info;
    info.mTime = doc.m_mTime;
    info.cTime = doc.m_cTime;

    static constexpr auto sizeOfInfo = sizeof(info);
    static constexpr auto sizeOfMTime = sizeof(doc.m_mTime);

    docTimeDB.put(id, info);
    m_approximatelyPendingData += sizeOfInfo;
    mtimeDB.put(doc.m_mTime, id);
    m_approximatelyPendingData += sizeOfMTime;

    if (!doc.m_data.isEmpty()) {
        docDataDB.put(id, doc.m_data);
        m_approximatelyPendingData += doc.m_data.size();
    }
}

QVector<QByteArray> WriteTransaction::addTerms(quint64 id, const QMap<QByteArray, Document::TermData>& terms)
{
    QVector<QByteArray> termList;
    termList.reserve(terms.size());
    m_pendingOperations.reserve(m_pendingOperations.size() + terms.size());

    QMapIterator<QByteArray, Document::TermData> it(terms);
    while (it.hasNext()) {
        const QByteArray& term = it.next().key();
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
    docTimeDB.del(id);
    mtimeDB.del(info.mTime, id);

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
        if (id) {
            removeRecursively(id);
        }
    }
    removeDocument(parentId);
}

bool WriteTransaction::removeRecursively(quint64 parentId, const std::function<bool(quint64)> &shouldDelete)
{
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    if (parentId && !shouldDelete(parentId)) {
        return false;
    }

    bool isEmpty = true;
    const QVector<quint64> children = docUrlDB.getChildren(parentId);
    for (quint64 id : children) {
        isEmpty &= removeRecursively(id, shouldDelete);
    }
    // refetch
    if (isEmpty && docUrlDB.getChildren(parentId).isEmpty()) {
        removeDocument(parentId);
        return true;
    }
    return false;
}

void WriteTransaction::replaceDocument(const Document& doc, DocumentOperations operations)
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    const quint64 id = doc.id();

    if (operations & DocumentTerms) {
        Q_ASSERT(!doc.m_terms.isEmpty());
        QVector<QByteArray> prevTerms = documentTermsDB.get(id);
        QVector<QByteArray> docTerms = replaceTerms(id, prevTerms, doc.m_terms);

        if (docTerms != prevTerms) {
            documentTermsDB.put(id, docTerms);
        }
    }

    if (operations & XAttrTerms) {
        QVector<QByteArray> prevTerms = documentXattrTermsDB.get(id);
        QVector<QByteArray> docXattrTerms = replaceTerms(id, prevTerms, doc.m_xattrTerms);

        if (docXattrTerms != prevTerms) {
            if (!docXattrTerms.isEmpty()) {
                documentXattrTermsDB.put(id, docXattrTerms);
            } else {
                documentXattrTermsDB.del(id);
            }
        }
    }

    if (operations & FileNameTerms) {
        QVector<QByteArray> prevTerms = documentFileNameTermsDB.get(id);
        QVector<QByteArray> docFileNameTerms = replaceTerms(id, prevTerms, doc.m_fileNameTerms);

        if (docFileNameTerms != prevTerms) {
            if (!docFileNameTerms.isEmpty()) {
                documentFileNameTermsDB.put(id, docFileNameTerms);
            } else {
                documentFileNameTermsDB.del(id);
            }
        }
    }

    if (doc.contentIndexing()) {
        contentIndexingDB.put(doc.id());
    }

    if (operations & DocumentTime) {
        DocumentTimeDB::TimeInfo info = docTimeDB.get(id);
        if (info.mTime != doc.m_mTime) {
            mtimeDB.del(info.mTime, id);
            mtimeDB.put(doc.m_mTime, id);
        }

        info.mTime = doc.m_mTime;
        info.cTime = doc.m_cTime;
        docTimeDB.put(id, info);
    }

    if (operations & DocumentData) {
        if (!doc.m_data.isEmpty()) {
            docDataDB.put(id, doc.m_data);
        } else {
            docDataDB.del(id);
        }
    }

    if (operations & DocumentUrl) {
        auto url = doc.url();
        int pos = url.lastIndexOf('/');
        auto newname = url.mid(pos + 1);
        docUrlDB.updateUrl(doc.id(), doc.parentId(), newname);
    }
}

QVector< QByteArray > WriteTransaction::replaceTerms(quint64 id, const QVector<QByteArray>& prevTerms,
                                                     const QMap<QByteArray, Document::TermData>& terms)
{
    m_pendingOperations.reserve(m_pendingOperations.size() + prevTerms.size() + terms.size());
    for (const QByteArray& term : prevTerms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }

    return addTerms(id, terms);
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
                sortedIdInsert(list, id);

                if (!op.data.positions.isEmpty()) {
                    if (!fetchedPositionList) {
                        positionList = positionDB.get(term);
                        fetchedPositionList = true;
                    }
                    sortedIdInsert(positionList, op.data);
                }
            }
            else {
                sortedIdRemove(list, id);
                if (!fetchedPositionList) {
                    positionList = positionDB.get(term);
                    fetchedPositionList = true;
                }
                sortedIdRemove(positionList, PositionInfo(id));
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

size_t Baloo::WriteTransaction::approximatelyPendingData() const
{
    return m_approximatelyPendingData;
}
