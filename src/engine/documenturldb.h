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

    /**
     * \p url new url of the file (complete path)
     */
    template <typename Functor>
    void replace(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder);

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
void DocumentUrlDB::replace(quint64 docId, const QByteArray& url, Functor shouldDeleteFolder)
{
    if (!docId || url.endsWith('/') || !url.startsWith('/')) {
        qWarning() << "replace called with invalid arguments, docId:" << docId << "url:" << url;
        return;
    }
    replaceOrDelete(docId, url, shouldDeleteFolder);
}

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
        const auto subDocs = idTreeDb.get(docId);
        if (!subDocs.isEmpty()) {
            // Check if subdocs actually exist or is it a corruption
            for (auto const& docId : subDocs) {
                auto filePath = idFilenameDb.get(docId);
                auto fileName = QFile::decodeName(filePath.name);
                if (QFile::exists(fileName)) {
                    if (!idTreeDb.get(docId).isEmpty()) {
                        qWarning() << "DocumentUrlDB::del"
                                   << "This folder still has sub-files in its cache. It cannot be deleted";
                    }
                } else {
                    /*
                     * FIXME: this is not an ideal solution we need to figure out how such corruptions are
                     * creeping in or at least if we detect some figure out a proper cleaning mechanism
                     */
                    qWarning() << "Database has corrupted entries baloo may misbehave, please recreate the DB by running $ balooctl disable && balooctl enable";
                }
            }
        }
        return;
    }

    put(docId, url);
}

}

#endif // BALOO_DOCUMENTURLDB_H
