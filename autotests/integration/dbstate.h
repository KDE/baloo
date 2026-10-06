/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DBSTATE_H
#define BALOO_DBSTATE_H

#include "transaction.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "documentiddb.h"
#include "documentdatadb.h"
#include "positiondb.h"
#include "documenttimedb.h"

namespace Baloo {

class DBState {
public:
    QMap<QByteArray, PostingList> postingDb;
    QMap<QByteArray, QVector<PositionInfo>> positionDb;

    QMap<quint64, QVector<QByteArray>> docTermsDb;
    QMap<quint64, QVector<QByteArray>> docFileNameTermsDb;
    QMap<quint64, QVector<QByteArray>> docXAttrTermsDb;

    QMap<quint64, DocumentTimeDB::TimeInfo> docTimeDb;
    QMap<quint32, quint64> mtimeDb;

    QMap<quint64, QByteArray> docDataDb;
    QMap<quint64, QByteArray> docUrlDb;
    QVector<quint64> contentIndexingDb;
    QVector<quint64> failedIdDb;

    bool operator== (const DBState& st) const {
        return postingDb == st.postingDb && positionDb == st.positionDb && docTermsDb == st.docTermsDb
               && docFileNameTermsDb == st.docFileNameTermsDb && docXAttrTermsDb == st.docXAttrTermsDb
               && docTimeDb == st.docTimeDb && mtimeDb == st.mtimeDb && docDataDb == st.docDataDb
               && docUrlDb == st.docUrlDb && contentIndexingDb == st.contentIndexingDb
               && failedIdDb == st.failedIdDb;
    }

    static DBState fromTransaction(Transaction* tr);
private:
};

DBState DBState::fromTransaction(Baloo::Transaction* tr)
{
    auto dbis = tr->m_dbis;
    MDB_txn* txn = tr->m_txn;

    PostingDB postingDB(dbis.postingDbi, txn);
    PositionDB positionDB(dbis.positionDBi, txn);
    DocumentDB documentTermsDB(dbis.docTermsDbi, txn);
    DocumentDB documentXattrTermsDB(dbis.docXattrTermsDbi, txn);
    DocumentDB documentFileNameTermsDB(dbis.docFilenameTermsDbi, txn);
    DocumentTimeDB docTimeDB(dbis.docTimeDbi, txn);
    DocumentDataDB docDataDB(dbis.docDataDbi, txn);
    DocumentIdDB contentIndexingDB(dbis.contentIndexingDbi, txn);
    DocumentIdDB failedIdDb(dbis.failedIdDbi, txn);
    MTimeDB mtimeDB(dbis.mtimeDbi, txn);
    DocumentUrlDB docUrlDB(dbis.idTreeDbi, dbis.idFilenameDbi, txn);

    DBState state;
    state.postingDb = postingDB.toTestMap();
    state.positionDb = positionDB.toTestMap();
    state.docTermsDb = documentTermsDB.toTestMap();
    state.docXAttrTermsDb = documentXattrTermsDB.toTestMap();
    state.docFileNameTermsDb = documentFileNameTermsDB.toTestMap();
    state.docTimeDb = docTimeDB.toTestMap();
    state.docDataDb = docDataDB.toTestMap();
    state.mtimeDb = mtimeDB.toTestMap();
    state.contentIndexingDb = contentIndexingDB.toTestVector();
    state.failedIdDb = failedIdDb.toTestVector();

    // FIXME: What about DocumentUrlDB?
    // state.docUrlDb = docUrlDB.toTestMap();

    return state;
}

/**
 * Helper for QTest
 * \sa QTest::toString
 */
inline char *toString(const DBState &state)
{
    QString buffer;
    QDebug stream(&buffer);
    stream << state.postingDb //
           << state.positionDb //
           << state.docTermsDb //
           << state.docFileNameTermsDb //
           << state.docXAttrTermsDb //
           << state.docTimeDb //
           << state.mtimeDb //
           << state.docDataDb //
           << state.docUrlDb //
           << state.contentIndexingDb //
           << state.failedIdDb;
    return qstrdup(buffer.toUtf8().constData());
}
} // namespace

#endif
