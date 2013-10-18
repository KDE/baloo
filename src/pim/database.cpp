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

Database::Database(QObject* parent)
    : QObject(parent)
    , m_plainTextDb(0)
{
}

Database::~Database()
{
    delete m_plainTextDb;
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

    if (!m_plainTextDb) {
        std::string path = m_pathDir.data() + std::string("text");
        m_plainTextDb = new Xapian::WritableDatabase(path, Xapian::DB_CREATE_OR_OPEN);
    }
}

void Database::endDocument()
{
    m_plainTextDb->replace_document(m_docId, m_plainTextDoc);

    m_plainTextDoc.clear_terms();
    m_plainTextDoc.clear_values();

    m_docId = 0;
}

void Database::insert(const QByteArray& key, const QByteArray& value)
{
    if (m_docId == 0) {
        kError() << "Attempting to insert without calling beginDocument";
        return;
    }

    Xapian::WritableDatabase* database;

    QHash<QByteArray, Xapian::WritableDatabase*>::iterator it = m_databases.find(key);
    if (it == m_databases.end()) {
        std::string path = m_pathDir.data() + std::string(key.data());
        database = new Xapian::WritableDatabase(path, Xapian::DB_CREATE_OR_OPEN);

        //it.value() = database;
        m_databases.insert(key, database);
    }
    else {
        database = it.value();
    }

    Xapian::Document doc;
    // FIXME: We do not want this!! Results in massive data duplication
    // How do we do not do this?
    //// doc.set_data(value.data());

    Xapian::TermGenerator termGen;
    termGen.set_document(doc);
    termGen.index_text_without_positions(value.data());

    database->replace_document(m_docId, doc);

    // Also insert in the 'text' document
    {
        Xapian::TermGenerator termGen;
        termGen.set_document(m_plainTextDoc);
        termGen.set_stopper(&m_stopper);
        termGen.index_text_without_positions(value.data());
    }
}

void Database::insertText(const QString& text)
{
    if (m_docId == 0) {
        kError() << "Attempting to insert without calling beginDocument";
        return;
    }

    Xapian::SimpleStopper stopper;

    Xapian::TermGenerator termGen;
    termGen.set_document(m_plainTextDoc);
    termGen.set_stopper(&m_stopper);
    termGen.index_text_without_positions(text.toUtf8().data());
}

void Database::insertBool(const QByteArray& key, bool value)
{
    if (m_docId == 0) {
        kError() << "Attempting to insert without calling beginDocument";
        return;
    }

    if (!value)
        return;

    std::string term = std::string("X") + key.data();
    m_plainTextDoc.add_boolean_term(term);
}

void Database::commit()
{
    QHash<QByteArray, Xapian::WritableDatabase*>::iterator it = m_databases.begin();
    for (; it != m_databases.end(); it++) {
        it.value()->commit();
    }
    m_plainTextDb->commit();
}
