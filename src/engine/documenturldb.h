/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTURLDB_H
#define BALOO_DOCUMENTURLDB_H

#include "idtreedb.h"
#include "idfilenamedb.h"
#include "idutils.h"

#include <QDebug>
#include <QFile>

namespace Baloo {

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
    bool contains(quint64 docId) const;

    /**
     * Move the document \p id to directory \p newParentId, set its name
     * to \p newName.
     */
    void updateUrl(quint64 id, quint64 newParentId, const QByteArray& newName);

    /**
     * Deletes a document from the DB, and conditionally also removes its
     * parent folders.
     *
     * \arg shouldDeleteFolder This function is called on any empty parent folder,
     * it is used to determine if that folder should be deleted.
     */
    template <typename Functor>
    void del(quint64 docId, Functor shouldDeleteFolder) {
        if (!docId) {
            qWarning() << "del called with invalid docId:" << docId;
            return;
        }
        replaceOrDelete(docId, shouldDeleteFolder);
    }

    quint64 getId(quint64 docId, const QByteArray& fileName) const;

    PostingIterator* iter(quint64 docId) {
        IdTreeDB db(m_idTreeDbi, m_txn);
        return db.iter(docId);
    }

    QMap<quint64, QByteArray> toTestMap() const;

private:
    BALOO_ENGINE_NO_EXPORT void add(quint64 id, quint64 parentId, const QByteArray& name);

    template <typename Functor>
    void replaceOrDelete(quint64 docId, Functor shouldDeleteFolder);

    MDB_txn* m_txn;
    MDB_dbi m_idFilenameDbi;
    MDB_dbi m_idTreeDbi;

    friend class UrlTest;
};

template <typename Functor>
void DocumentUrlDB::replaceOrDelete(quint64 docId, Functor shouldDeleteFolder)
{
    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    // FIXME: Maybe this can be combined into one?
    auto path = idFilenameDb.get(docId);
    if (path.name.isEmpty()) {
        return;
    }

    idFilenameDb.del(docId);

    QVector<quint64> subDocs = idTreeDb.get(path.parentId);
    subDocs.removeOne(docId);
    idTreeDb.set(path.parentId, subDocs);

    if (subDocs.isEmpty()) {
        //
        // Delete every parent directory which only has 1 child
        //
        quint64 id = path.parentId;
        while (id) {
            auto path = idFilenameDb.get(id);
            // FIXME: Prevents database cleaning
            // Q_ASSERT(!path.name.isEmpty());

            subDocs = idTreeDb.get(path.parentId);
            if (subDocs.size() == 1 && shouldDeleteFolder(id)) {
                idTreeDb.set(path.parentId, {});
                idFilenameDb.del(id);
            } else {
                break;
            }

            id = path.parentId;
        }
    }
}

}

#endif // BALOO_DOCUMENTURLDB_H
