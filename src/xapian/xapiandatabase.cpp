/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "xapiandatabase.h"
#include "xapiandocument.h"

#include <KDebug>
#include <QTimer>
#include <QDir>

#include <malloc.h>

using namespace Baloo;

XapianDatabase::XapianDatabase(const QString& path)
    : QObject()
    , m_db(0)
{
    QDir().mkpath(path);
    m_path = path.toUtf8().constData();

    try {
        Xapian::WritableDatabase(m_path, Xapian::DB_CREATE_OR_OPEN);
        m_db = new Xapian::Database(m_path);
    }
    catch (const Xapian::DatabaseError& err) {
        kError() << "Serious Error: " << err.get_error_string();
        kError() << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
    }

    // Possible errors - DatabaseLock error
    // Corrupt and InvalidID error
}

void XapianDatabase::replaceDocument(uint id, const Xapian::Document& doc)
{
    m_docsToAdd << qMakePair(id, doc);
}

void XapianDatabase::deleteDocument(uint id)
{
    m_docsToRemove << id;
}

void XapianDatabase::commit()
{
    if (m_docsToAdd.isEmpty() && m_docsToRemove.isEmpty()) {
        Q_EMIT committed();
        return;
    }

    try {
        Xapian::WritableDatabase wdb(m_path, Xapian::DB_CREATE_OR_OPEN);

        kDebug() << "Adding:" << m_docsToAdd.size() << "docs";
        Q_FOREACH (const DocIdPair& doc, m_docsToAdd) {
            wdb.replace_document(doc.first, doc.second);
        }

        kDebug() << "Removing:" << m_docsToRemove.size() << "docs";
        Q_FOREACH (Xapian::docid id, m_docsToRemove) {
            try {
                wdb.delete_document(id);
            }
            catch (const Xapian::DocNotFoundError&) {
            }
        }

        wdb.commit();
        m_db->reopen();
        kDebug() << "Xapian Committed";

        m_docsToAdd.clear();
        m_docsToRemove.clear();

        malloc_trim(0);

        Q_EMIT committed();
    }
    catch (const Xapian::DatabaseLockError& err) {
        kError() << err.get_msg().c_str();
        retryCommit();
    }
    catch (const Xapian::DatabaseModifiedError& err) {
        kError() << err.get_msg().c_str();
        kError() << "Commit failed, retrying in another 200 msecs";
        retryCommit();
    }
    catch (const Xapian::DatabaseError& err) {
        kError() << err.get_msg().c_str();
        retryCommit();
    }
}

void XapianDatabase::retryCommit()
{
    QTimer::singleShot(200, this, SLOT(commit()));
}

XapianDocument XapianDatabase::document(uint id)
{
    try {
        Xapian::Document xdoc = m_db->get_document(id);
        return XapianDocument(xdoc);
    }
    catch (const Xapian::DatabaseModifiedError&) {
        m_db->reopen();
        return document(id);
    }
    catch (const Xapian::Error&) {
        return XapianDocument();
    }
}



