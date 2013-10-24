/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "database.h"

#include <KDebug>
#include <QStringList>

Database::Database()
{
}

Database::~Database()
{
    qDeleteAll(m_documents);
    qDeleteAll(m_databases);
}

void Database::setPath(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    if (path[path.length()-1] == '/') {
        m_pathDir = path.toUtf8();
    }
    else {
        m_pathDir.clear();
        m_pathDir.append('/');
        m_pathDir.append(path.toUtf8());
    }
}

QString Database::path() const {
    return QString::fromUtf8(m_pathDir);
}

void Database::beginDocument(uint id)
{
    m_docId = id;
}

void Database::endDocument()
{
    QHash<Xapian::WritableDatabase*, Xapian::Document*>::iterator it = m_documents.begin();
    for (; it != m_documents.end(); it++) {
        Xapian::WritableDatabase* db = it.key();
        Xapian::Document* doc = it.value();

        db->replace_document(m_docId, *doc);
        delete doc;
    }
    m_documents.clear();

    m_docId = 0;
}

Xapian::WritableDatabase* Database::fetchDb(const QByteArray& key)
{
    Q_ASSERT_X(m_docId != 0, "Database", "Attempting to insert data without calling beginDocument");
    Q_ASSERT_X(key.size(), "Database", "Attempting to insert data with an empty key");

    QHash<QByteArray, Xapian::WritableDatabase*>::iterator it = m_databases.find(key);
    if (it == m_databases.end()) {
        std::string path = m_pathDir.data() + std::string(key.data());

        Xapian::WritableDatabase* db = new Xapian::WritableDatabase(path, Xapian::DB_CREATE_OR_OPEN);
        m_databases.insert(key, db);
        return db;
    }
    else {
        return it.value();
    }
}

void Database::set(const QByteArray& key, const QByteArray& value)
{
    Xapian::WritableDatabase* db = fetchDb(key);

    Xapian::Document doc;
    doc.add_term(value.data());
    db->replace_document(m_docId, doc);
}

void Database::setText(const QByteArray& key, const QByteArray& value)
{
    Xapian::WritableDatabase* db = fetchDb(key);

    Xapian::Document doc;
    Xapian::TermGenerator termGen;
    termGen.set_document(doc);
    termGen.index_text_without_positions(value.data());

    db->replace_document(m_docId, doc);
}

Xapian::Document* Database::fetchDoc(Xapian::WritableDatabase* db)
{
    QHash<Xapian::WritableDatabase*, Xapian::Document*>::const_iterator it = m_documents.constFind(db);
    if (it == m_documents.constEnd()) {
        Xapian::Document* doc = new Xapian::Document();

        m_documents.insert(db, doc);
        return doc;
    }
    else {
        return it.value();
    }
}

void Database::append(const QByteArray& key, const QByteArray& value)
{
    Xapian::WritableDatabase* db = fetchDb(key);
    Xapian::Document* doc = fetchDoc(db);
    doc->add_term(value.data());
}

void Database::appendText(const QByteArray& key, const QByteArray& value)
{
    Xapian::WritableDatabase* db = fetchDb(key);
    Xapian::Document* doc = fetchDoc(db);

    Xapian::TermGenerator termGen;
    termGen.set_document(*doc);
    termGen.index_text_without_positions(value.data());
}

void Database::appendBool(const QByteArray& dbKey, const QByteArray& key, bool value)
{
    if (!value)
        return;

    Xapian::WritableDatabase* db = fetchDb(dbKey);
    Xapian::Document* doc = fetchDoc(db);

    std::string term = std::string("X") + key.data();
    doc->add_boolean_term(term);
}

void Database::commit()
{
    QHash<QByteArray, Xapian::WritableDatabase*>::iterator it = m_databases.begin();
    for (; it != m_databases.end(); it++) {
        it.value()->commit();
    }
}
