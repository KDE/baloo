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
    termGen.index_text(value.data());

    database->replace_document(m_docId, doc);

    {
        Xapian::TermGenerator termGen;
        termGen.set_document(doc);
        termGen.index_text(value.data());

        m_plainTextDb->replace_document(m_docId, doc);
    }
}

void Database::insertText(const QString& text)
{
    if (m_docId == 0) {
        kError() << "Attempting to insert without calling beginDocument";
        return;
    }

    Xapian::SimpleStopper stopper;

    Xapian::Document doc;
    Xapian::TermGenerator termGen;
    termGen.set_document(doc);
    termGen.set_stopper(&stopper);
    termGen.index_text(text.toUtf8().data());

    m_plainTextDb->replace_document(m_docId, doc);
}

/*
void Database::test()
{
    Xapian::Database databases;
    bool isEmpty = true;
    QHash< QByteArray, Xapian::WritableDatabase* >::iterator it = m_databases.begin();
    for (; it != m_databases.end(); it++) {
        Xapian::WritableDatabase* db = it.value();
        db->commit();
        databases.add_database(*db);

        isEmpty = false;
    }

    if (isEmpty) {
        kDebug() << "No db found";
        return;
    }

    Xapian::Enquire enquire(databases);
    //Xapian::Query query("aleix");

    Xapian::QueryParser queryParser;
    queryParser.set_database(databases);
    Xapian::Query query = queryParser.parse_query("running", Xapian::QueryParser::FLAG_PARTIAL);

    kDebug() << "Query:" << query.get_description().c_str();
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10);
    Xapian::MSetIterator i;
    for (i = matches.begin(); i != matches.end(); ++i) {
        kDebug() << "Document ID " << *i << "\t";
        kDebug() << "Document ID " << i.get_document().get_docid() << "\t";
        kDebug() << i.get_percent() << "% ";
        Xapian::Document doc = i.get_document();
        kDebug() << "[" << doc.get_data().c_str() << "]" << endl;
    }
}*/

void Database::commit()
{
    QHash<QByteArray, Xapian::WritableDatabase*>::iterator it = m_databases.begin();
    for (; it != m_databases.end(); it++) {
        it.value()->commit();
    }
    m_plainTextDb->commit();
}


/*
int main(int argc, char** argv) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    /*
    Database db;
    db.insert(10, "subject", "This is the email run subject");
    db.insert(10, "from", "Visehsh Handa <me@vhanda.in>");
    db.insert(10, "cc", "Aleix Pol <apol@kde.org>");
    db.insert(10, "to", "Alex Fiestas <afiestas@kde.org>");
    db.insert(10, "content", "This is the content of the email!");

    try {
        db.test();
    }
    catch (const Xapian::InvalidArgumentError& err) {
        kDebug() << err.get_description().c_str();
        kDebug() << err.get_context().c_str();
    }*/

//    return 0;
//}
