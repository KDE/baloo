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

#include "database.h"
#include "postingdb.h"
#include "documentdb.h"
#include "documenturldb.h"
#include "urldocumentdb.h"

#include "document.h"

#include "andpostingiterator.h"

#include <QFile>

using namespace Baloo;

Database::Database(const QString& path)
{
    mdb_env_create(&m_env);
    mdb_env_set_maxdbs(m_env, 4);

    // The directory needs to be created before opening the environment
    QByteArray arr = QFile::encodeName(path);
    mdb_env_open(m_env, arr.constData(), 0, 0664);
    mdb_txn_begin(m_env, NULL, 0, &m_txn);

    m_postingDB = new PostingDB(m_txn);
    m_documentDB = new DocumentDB(m_txn);
    m_docUrlDB = new DocumentUrlDB(m_txn);
    m_urlDocDB = new UrlDocumentDB(m_txn);
}

Database::~Database()
{
    delete m_postingDB;
    delete m_documentDB;
    delete m_docUrlDB;
    delete m_urlDocDB;

    mdb_txn_commit(m_txn);
    mdb_env_close(m_env);
}

void Database::addDocument(const Document& doc)
{
    Q_ASSERT(doc.id() > 0);

    const uint id = doc.id();
    for (const QByteArray& term : doc.m_terms) {
        PostingList list = m_postingDB->get(term);
        list << id;

        m_postingDB->put(term, list);
    }

    m_documentDB->put(id, doc.m_terms);
    m_docUrlDB->put(id, doc.url());
    m_urlDocDB->put(doc.url(), id);
}

void Database::removeDocument(uint id)
{
    Q_ASSERT(id > 0);

    QVector<QByteArray> terms = m_documentDB->get(id);
    if (terms.isEmpty()) {
        return;
    }

    for (const QByteArray& term : terms) {
        PostingList list = m_postingDB->get(term);
        list.removeOne(id);

        m_postingDB->put(term, list);
    }

    m_documentDB->del(id);

    QByteArray url = m_docUrlDB->get(id);
    m_docUrlDB->del(id);
    m_urlDocDB->del(url);
}

bool Database::hasDocument(uint id)
{
    // FIXME: Find a better way!
    return (document(id).id() == id);
}

Document Database::document(uint id)
{
    Q_ASSERT(id > 0);

    QVector<QByteArray> terms = m_documentDB->get(id);
    if (terms.isEmpty()) {
        return Document();
    }

    Document doc;
    doc.m_id = id;
    doc.m_terms = terms;
    doc.m_url = m_docUrlDB->get(id);

    return doc;
}

uint Database::documentId(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());
    return m_urlDocDB->get(url);
}

QByteArray Database::documentUrl(uint id)
{
    Q_ASSERT(id > 0);
    return m_docUrlDB->get(id);
}

void Database::commit()
{
    int rc = mdb_txn_commit(m_txn);
    Q_ASSERT(rc == 0);
}

QVector<int> Database::exec(const QVector<QByteArray>& query)
{
    Q_ASSERT(!query.isEmpty());

    QVector<PostingIterator*> list;
    list.reserve(query.size());

    for (const QByteArray& term : query) {
        PostingIterator* iter = m_postingDB->iter(term);
        if (iter) {
            list << iter;
        }
    }

    if (list.isEmpty()) {
        return QVector<int>();
    }

    QVector<int> result;
    if (list.size() == 1) {
        PostingIterator* it = list.first();
        while (it->next()) {
            result << it->docId();
        }
        return result;
    }

    AndPostingIterator it(list);
    while (it.next()) {
        result << it.docId();
    }

    return result;
}


