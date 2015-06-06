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
    static bool debugCompare(const DBState& st1, const DBState& st2);
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

bool DBState::debugCompare(const DBState& st1, const DBState& st2)
{
    if (st1.postingDb != st2.postingDb) {
        qDebug() << "Posting DB different";
        qDebug() << st1.postingDb;
        qDebug() << st2.postingDb;
        return false;
    }

    if (st1.positionDb != st2.positionDb) {
        qDebug() << "Position DB different";
        return false;
    }

    if (st1.docTermsDb != st2.docTermsDb) {
        qDebug() << "DocTerms DB different";
        qDebug() << st1.docTermsDb;
        qDebug() << st2.docTermsDb;
        return false;
    }

    if (st1.docFileNameTermsDb != st2.docFileNameTermsDb) {
        qDebug() << "Doc FileName Terms DB different";
        return false;
    }

    return st1 == st2;
}
} // namespace

#endif
