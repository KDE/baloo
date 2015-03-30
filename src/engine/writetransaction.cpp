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

#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"

using namespace Baloo;

WriteTransaction::WriteTransaction(PostingDB* postingDB, PositionDB* positionDB,
                                   DocumentDB* docTerms, DocumentDB* docXattrTerms, DocumentDB* docFileNameTerms,
                                   DocumentUrlDB* docUrlDB, DocumentTimeDB* docTimeDB,
                                   DocumentDataDB* docDataDB, DocumentIdDB* contentIndexingDB)
    : m_postingDB(postingDB)
    , m_positionDB(positionDB)
    , m_documentTermsDB(docTerms)
    , m_documentXattrTermsDB(docXattrTerms)
    , m_documentFileNameTermsDB(docFileNameTerms)
    , m_docUrlDB(docUrlDB)
    , m_docTimeDB(docTimeDB)
    , m_docDataDB(docDataDB)
    , m_contentIndexingDB(contentIndexingDB)
{
}

void WriteTransaction::addDocument(const Document& doc)
{
    quint64 id = doc.id();

    QVector<QByteArray> docTerms;
    docTerms.reserve(doc.m_terms.size());

    // FIXME: Add asserts to make sure the document does not already exist
    //        Otherwise we will have strange stale data
    QMapIterator<QByteArray, Document::TermData> it(doc.m_terms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    m_documentTermsDB->put(id, docTerms);

    QVector<QByteArray> docXattrTerms;
    docXattrTerms.reserve(doc.m_xattrTerms.size());

    it = QMapIterator<QByteArray, Document::TermData>(doc.m_xattrTerms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docXattrTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    if (!docXattrTerms.isEmpty())
        m_documentXattrTermsDB->put(id, docXattrTerms);

    QVector<QByteArray> docFileNameTerms;
    docFileNameTerms.reserve(doc.m_fileNameTerms.size());

    it = QMapIterator<QByteArray, Document::TermData>(doc.m_fileNameTerms);
    while (it.hasNext()) {
        const QByteArray term = it.next().key();
        docFileNameTerms.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    if (!docFileNameTerms.isEmpty())
        m_documentFileNameTermsDB->put(id, docFileNameTerms);

    if (!doc.url().isEmpty()) {
        m_docUrlDB->put(id, doc.url());
    }

    if (doc.contentIndexing()) {
        m_contentIndexingDB->put(doc.id());
    }

    DocumentTimeDB::TimeInfo info;
    info.mTime = doc.m_mTime;
    info.cTime = doc.m_cTime;
    info.julianDay = doc.m_julianDay;

    m_docTimeDB->put(id, info);

    if (!doc.m_data.isEmpty()) {
        m_docDataDB->put(id, doc.m_data);
    }
}

void WriteTransaction::removeDocument(quint64 id)
{
    // FIXME: Optimize this. We do not need to combine them into one big vector
    QVector<QByteArray> terms = m_documentTermsDB->get(id) + m_documentXattrTermsDB->get(id) + m_documentFileNameTermsDB->get(id);
    if (terms.isEmpty()) {
        return;
    }

    for (const QByteArray& term : terms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }

    m_documentTermsDB->del(id);
    m_documentXattrTermsDB->del(id);
    m_documentFileNameTermsDB->del(id);

    m_docUrlDB->del(id);

    m_contentIndexingDB->del(id);
    m_docTimeDB->del(id);
    m_docDataDB->del(id);
}

void WriteTransaction::replaceDocument(const Document& doc, Database::DocumentOperations operations)
{
    const quint64 id = doc.id();

    if (operations & Database::DocumentTerms) {
        QVector<QByteArray> prevTerms = m_documentTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docTerms;
        docTerms.reserve(doc.m_terms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_terms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        m_documentTermsDB->put(id, docTerms);
    }

    if (operations & Database::XAttrTerms) {
        QVector<QByteArray> prevTerms = m_documentXattrTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docXattrTerms;
        docXattrTerms.reserve(doc.m_xattrTerms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_xattrTerms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docXattrTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        if (!docXattrTerms.isEmpty())
            m_documentXattrTermsDB->put(id, docXattrTerms);
    }

    if (operations & Database::FileNameTerms) {
        QVector<QByteArray> prevTerms = m_documentFileNameTermsDB->get(id);
        for (const QByteArray& term : prevTerms) {
            Operation op;
            op.type = RemoveId;
            op.data.docId = id;

            m_pendingOperations[term].append(op);
        }

        QVector<QByteArray> docFileNameTerms;
        docFileNameTerms.reserve(doc.m_fileNameTerms.size());

        QMapIterator<QByteArray, Document::TermData> it(doc.m_fileNameTerms);
        while (it.hasNext()) {
            const QByteArray term = it.next().key();
            docFileNameTerms.append(term);

            Operation op;
            op.type = AddId;
            op.data.docId = id;
            op.data.positions = it.value().positions;

            m_pendingOperations[term].append(op);
        }

        if (!docFileNameTerms.isEmpty())
            m_documentFileNameTermsDB->put(id, docFileNameTerms);
    }

    if (operations & Database::DocumentUrl) {
        // FIXME: Replacing the documentUrl is actually quite complicated!
        Q_ASSERT(0);
        m_docUrlDB->put(id, doc.url());
    }

    // FIXME: What about contentIndexing?
    /*
    if (doc.contentIndexing()) {
        m_contentIndexingDB->put(doc.id());
    }
    */

    if (operations & Database::DocumentTime) {
        DocumentTimeDB::TimeInfo info;
        info.mTime = doc.m_mTime;
        info.cTime = doc.m_cTime;
        info.julianDay = doc.m_julianDay;

        m_docTimeDB->put(id, info);
    }

    if (operations & Database::DocumentData) {
        m_docDataDB->put(id, doc.m_data);
    }
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

void WriteTransaction::commit()
{
    qDebug() << "PendingOperations:" << m_pendingOperations.size();
    QHashIterator<QByteArray, QVector<Operation> > iter(m_pendingOperations);
    while (iter.hasNext()) {
        iter.next();

        const QByteArray& term = iter.key();
        const QVector<Operation> operations = iter.value();

        PostingList list = m_postingDB->get(term);
        QVector<PositionInfo> positionList = m_positionDB->get(term); // FIXME: We do not need to fetch this for all the terms

        for (const Operation& op : operations) {
            quint64 id = op.data.docId;

            if (op.type == AddId) {
                insert(list, id);

                if (!op.data.positions.isEmpty()) {
                    insert(positionList, op.data);
                }
            }
            else {
                list.removeOne(id);
                positionList.removeOne(PositionInfo(id));
            }
        }

        m_postingDB->put(term, list);
        if (!positionList.isEmpty()) {
            m_positionDB->put(term, positionList);
        }
    }

    m_pendingOperations.clear();
}
