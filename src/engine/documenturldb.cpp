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

#include "documenturldb.h"
#include "idutils.h"
#include "postingiterator.h"

#include <algorithm>
#include <qplatformdefs.h>
#include <QPair>
#include <QDebug>

using namespace Baloo;

DocumentUrlDB::DocumentUrlDB(MDB_txn* txn)
    : m_idFilename(txn)
    , m_idTree(txn)
{
}

DocumentUrlDB::~DocumentUrlDB()
{
}

void DocumentUrlDB::put(quint64 docId, const QByteArray& url)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!url.isEmpty());
    Q_ASSERT(!url.endsWith('/'));

    typedef QPair<quint64, QByteArray> IdNamePath;

    QByteArray arr = url;
    //
    // id and parent
    //
    {
        quint64 id = filePathToId(arr);
        Q_ASSERT(id == docId);

        int pos = arr.lastIndexOf('/');
        QByteArray name = arr.mid(pos + 1);

        arr.resize(pos);
        quint64 parentId = filePathToId(arr);

        add(id, parentId, name);
        if (m_idFilename.contains(parentId))
            return;
    }

    //
    // The rest of the path
    //
    QVector<IdNamePath> list;
    while (!arr.isEmpty()) {
        quint64 id = filePathToId(arr);

        int pos = arr.lastIndexOf('/');
        QByteArray name = arr.mid(pos + 1);

        list.prepend(qMakePair(id, name));
        arr.resize(pos);
    }

    for (int i = 0; i < list.size(); i++) {
        quint64 id = list[i].first;
        QByteArray name = list[i].second;

        // Update the IdTree
        quint64 parentId = 0;
        if (i) {
            parentId = list[i-1].first;
        }

        add(id, parentId, name);
    }
}

void DocumentUrlDB::add(quint64 id, quint64 parentId, const QByteArray& name)
{
    QVector<quint64> subDocs = m_idTree.get(parentId);

    // Find if the id exists
    if (subDocs.isEmpty()) {
        subDocs.append(id);
    } else {
        auto it = std::upper_bound(subDocs.begin(), subDocs.end(), id);

        // Merge the id if it does not
        auto prev = it - 1;
        if (*prev != id) {
            subDocs.insert(it, id);
        }
    }

    m_idTree.put(parentId, subDocs);

    // Update the IdFileName
    IdFilenameDB::FilePath path;
    path.parentId = parentId;
    path.name = name;

    m_idFilename.put(id, path);
}

QByteArray DocumentUrlDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    auto path = m_idFilename.get(docId);
    if (path.name.isEmpty()) {
        return QByteArray();
    }

    QList<QByteArray> list = {path.name};
    quint64 id = path.parentId;
    while (id) {
        auto p = m_idFilename.get(id);
        Q_ASSERT(!p.name.isEmpty());

        list.prepend(p.name);
        id = p.parentId;
    }

    return '/' + list.join('/');
}

void DocumentUrlDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    // FIXME: Maybe this can be combined into one?
    auto path = m_idFilename.get(docId);
    if (path.name.isEmpty()) {
        return;
    }
    m_idFilename.del(docId);

    QVector<quint64> subDocs = m_idTree.get(path.parentId);
    subDocs.removeOne(docId);

    if (!subDocs.isEmpty()) {
        m_idTree.put(path.parentId, subDocs);
    } else {
        m_idTree.del(path.parentId);

        //
        // Delete every parent directory which only has 1 child
        //
        quint64 id = path.parentId;
        while (id) {
            auto path = m_idFilename.get(id);
            Q_ASSERT(!path.name.isEmpty());
            QVector<quint64> subDocs = m_idTree.get(path.parentId);
            if (subDocs.size() == 1) {
                m_idTree.del(path.parentId);
                m_idFilename.del(id);
            } else {
                break;
            }

            id = path.parentId;
        }
    }
}

void DocumentUrlDB::rename(quint64 docId, const QByteArray& newFileName)
{
    Q_ASSERT(docId > 0);

    auto path = m_idFilename.get(docId);
    path.name = newFileName;
    m_idFilename.put(docId, path);
}

quint64 DocumentUrlDB::getId(quint64 docId, const QByteArray& fileName)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!fileName.isEmpty());

    QVector<quint64> subFiles = m_idTree.get(docId);
    for (quint64 id : subFiles) {
        IdFilenameDB::FilePath path = m_idFilename.get(id);
        if (path.name == fileName) {
            return id;
        }
    }

    return 0;
}

