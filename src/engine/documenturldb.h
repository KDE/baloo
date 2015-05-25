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

#ifndef BALOO_DOCUMENTURLDB_H
#define BALOO_DOCUMENTURLDB_H

#include "idtreedb.h"
#include "idfilenamedb.h"

namespace Baloo {

class UrlTest;
class PostingIterator;

class BALOO_ENGINE_EXPORT DocumentUrlDB
{
public:
    explicit DocumentUrlDB(MDB_dbi idTreeDb, MDB_dbi idFileNameDb, MDB_txn* txn);
    ~DocumentUrlDB();

    void put(quint64 docId, const QByteArray& url);
    QByteArray get(quint64 docId) const;
    QVector<quint64> getChildren(quint64 docId) const;

    void del(quint64 docId);
    /**
     * If \p url is empty then the docId is deleted
     */
    void replace(quint64 docId, const QByteArray& url);
    void rename(quint64 docId, const QByteArray& newFileName);

    quint64 getId(quint64 docId, const QByteArray& fileName) const;

    PostingIterator* iter(quint64 docId) {
        IdTreeDB db(m_idTreeDbi, m_txn);
        return db.iter(docId);
    }

    QMap<quint64, QByteArray> toTestMap() const;

private:
    void add(quint64 id, quint64 parentId, const QByteArray& name);

    MDB_txn* m_txn;
    MDB_dbi m_idFilenameDbi;
    MDB_dbi m_idTreeDbi;

    friend class UrlTest;
};

}

#endif // BALOO_DOCUMENTURLDB_H
