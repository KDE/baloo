/*
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

//#include "config.h"

#include "xapiandatabase.h"
#include "xapiandocument.h"

#include <QDebug>
#include <QTimer>
#include <QDir>

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

using namespace Baloo;

XapianDatabase::XapianDatabase(const QString& path, bool writeOnly)
    : m_db(0)
    , m_writeOnly(writeOnly)
{
    QDir().mkpath(path);
    m_path = path.toUtf8().constData();

    if (!writeOnly) {
        try {
            createWritableDb();
            m_db = new Xapian::Database(m_path);
        }
        catch (const Xapian::DatabaseError& err) {
            qWarning() << "Serious Error: " << err.get_error_string();
            qWarning() << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        }

        // Possible errors - DatabaseLock error
        // Corrupt and InvalidID error
    }
    else {
        m_wDb = createWritableDb();
    }
}

XapianDatabase::~XapianDatabase()
{
    delete m_db;
}

void XapianDatabase::replaceDocument(uint id, const XapianDocument& doc)
{
    replaceDocument(id, doc.doc());
}

void XapianDatabase::replaceDocument(uint id, const Xapian::Document& doc)
{
    if (m_writeOnly) {
        try {
            m_wDb.replace_document(id, doc);
        }
        catch (const Xapian::Error&) {
        }
        return;
    }
    m_docsToAdd << qMakePair(id, doc);
}

void XapianDatabase::deleteDocument(uint id)
{
    if (m_writeOnly) {
        try {
            m_wDb.delete_document(id);
        }
        catch (const Xapian::Error&) {
        }
        return;
    }
    m_docsToRemove << id;
}

void XapianDatabase::commit()
{
    if (m_writeOnly) {
        try {
            m_wDb.commit();
        }
        catch (const Xapian::Error& err) {
            qWarning() << err.get_error_string();
        }
        return;
    }

    if (m_docsToAdd.isEmpty() && m_docsToRemove.isEmpty()) {
        return;
    }

    Xapian::WritableDatabase wdb = createWritableDb();

    qDebug() << "Adding:" << m_docsToAdd.size() << "docs";
    Q_FOREACH (const DocIdPair& doc, m_docsToAdd) {
        try {
            wdb.replace_document(doc.first, doc.second);
        }
        catch (const Xapian::Error&) {
        }
    }

    qDebug() << "Removing:" << m_docsToRemove.size() << "docs";
    Q_FOREACH (Xapian::docid id, m_docsToRemove) {
        try {
            wdb.delete_document(id);
        }
        catch (const Xapian::Error&) {
        }
    }

    try {
        wdb.commit();
        m_db->reopen();
    }
    catch (const Xapian::Error& err) {
        qWarning() << err.get_error_string();
    }
    qDebug() << "Xapian Committed";

    m_docsToAdd.clear();
    m_docsToRemove.clear();

#if defined(HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif
}

XapianDocument XapianDatabase::document(uint id)
{
    try {
        Xapian::Document xdoc;
        if (m_writeOnly) {
            xdoc = m_wDb.get_document(id);
        } else {
            xdoc = m_db->get_document(id);
        }
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

Xapian::WritableDatabase XapianDatabase::createWritableDb()
{
    // We need to keep sleeping for a required amount, until we reach
    // a threshold. That's when we decide to abort?
    for (int i = 1; i <= 20; i++) {
        try {
            Xapian::WritableDatabase wdb(m_path, Xapian::DB_CREATE_OR_OPEN);
            return wdb;
        }
        catch (const Xapian::DatabaseLockError&) {
            usleep(i * 50 * 1000);
        }
        catch (const Xapian::DatabaseModifiedError&) {
            usleep(i * 50 * 1000);
        }
        catch (const Xapian::DatabaseCreateError& err) {
            qDebug() << err.get_error_string();
            return Xapian::WritableDatabase();
        }
        catch (const Xapian::DatabaseCorruptError& err) {
            qWarning() << "Database Corrupted - What did you do?";
            qWarning() << err.get_error_string();
            return Xapian::WritableDatabase();
        }
        catch (...) {
            qWarning() << "Bananana Error";
            return Xapian::WritableDatabase();
        }
    }

    qWarning() << "Could not obtain lock for Xapian Database. This is bad";
    return Xapian::WritableDatabase();
}

