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

    /**
     * Returns true if added
     * Returns false is the file no longer exists and could not be added
     */
    bool put(quint64 docId, const QByteArray& url);

    QByteArray get(quint64 docId) const;
    QVector<quint64> getChildren(quint64 docId) const;

    /**
     * \arg shouldDeleteFolder This function is called on any empty folder, and is used to
     * determine if that empty folder should be deleted.
     */
    template <typename Functor>
    void del(quint64 docId, Functor shouldDeleteFolder) {
        replace(docId, QByteArray(), shouldDeleteFolder);
    }

    /**
     * If \p url is empty then the docId is deleted
     */
    template <typename Functor>
    void replace(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder);

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


template <typename Functor>
void DocumentUrlDB::replace(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder)
{
    Q_ASSERT(docId > 0);
    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    // FIXME: Maybe this can be combined into one?
    auto path = idFilenameDb.get(docId);
    if (path.name.isEmpty()) {
        return;
    }
    idFilenameDb.del(docId);

    QVector<quint64> subDocs = idTreeDb.get(path.parentId);
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    const int docIdIndex = subDocs.indexOf(docId);
    if (docIdIndex >= 0) {
        subDocs.remove(docIdIndex);
    }
#else
    subDocs.removeOne(docId);
#endif

    if (!subDocs.isEmpty()) {
        idTreeDb.put(path.parentId, subDocs);
    } else {
        idTreeDb.del(path.parentId);

        //
        // Delete every parent directory which only has 1 child
        //
        quint64 id = path.parentId;
        while (id) {
            auto path = idFilenameDb.get(id);
            Q_ASSERT(!path.name.isEmpty());

            QVector<quint64> subDocs = idTreeDb.get(path.parentId);
            if (subDocs.size() == 1 && shouldDeleteFolder(id)) {
                idTreeDb.del(path.parentId);
                idFilenameDb.del(id);
            } else {
                break;
            }

            id = path.parentId;
        }
    }

    if (url.isEmpty()) {
        Q_ASSERT_X(idTreeDb.get(docId).isEmpty(), "DocumentUrlDB::del", "This folder still has sub-files in its cache. It cannot be deleted");
        return;
    }

    put(docId, url);
}

}

#endif // BALOO_DOCUMENTURLDB_H
