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
     */
    void updateUrl(quint64 id, quint64 parentId, const QByteArray& url);

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
        replaceOrDelete(docId, QByteArray(), shouldDeleteFolder);
    }

    quint64 getId(quint64 docId, const QByteArray& fileName) const;

    PostingIterator* iter(quint64 docId) {
        IdTreeDB db(m_idTreeDbi, m_txn);
        return db.iter(docId);
    }

    QMap<quint64, QByteArray> toTestMap() const;

private:
    void add(quint64 id, quint64 parentId, const QByteArray& name);

    template <typename Functor>
    void replaceOrDelete(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder);

    MDB_txn* m_txn;
    MDB_dbi m_idFilenameDbi;
    MDB_dbi m_idTreeDbi;

    friend class UrlTest;
};

template <typename Functor>
void DocumentUrlDB::replaceOrDelete(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder)
{
    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    // FIXME: Maybe this can be combined into one?
    auto path = idFilenameDb.get(docId);
    if (path.name.isEmpty()) {
        return;
    }

    // Check for trivial case - simple rename, i.e. parent stays the same
    if (!url.isEmpty()) {
        auto lastSlash = url.lastIndexOf('/');
        auto parentPath = url.left(lastSlash + 1);
        auto parentId = filePathToId(parentPath);
        if (!parentId) {
            qDebug() << "parent" << parentPath << "of" << url << "does not exist";
            return;

        } else if (parentId == path.parentId) {
            auto newname = url.mid(lastSlash + 1);
            if (newname != path.name) {
                qDebug() << docId << url << "renaming" << path.name << "to" << newname;
                path.name = newname;
                idFilenameDb.put(docId, path);
            }
            return;
        }
    }

    idFilenameDb.del(docId);

    QVector<quint64> subDocs = idTreeDb.get(path.parentId);
    subDocs.removeOne(docId);

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
            // FIXME: Prevents database cleaning
            // Q_ASSERT(!path.name.isEmpty());

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
        // Delete case, nothing more to do
        return;
    }

    put(docId, url);
}

}

#endif // BALOO_DOCUMENTURLDB_H
