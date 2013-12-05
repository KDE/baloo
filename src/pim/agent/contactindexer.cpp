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

#include "contactindexer.h"

#include <KABC/Addressee>

ContactIndexer::ContactIndexer(const QString& path):
    AbstractIndexer()
{
    m_db = new Xapian::WritableDatabase(path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
}

ContactIndexer::~ContactIndexer()
{
    m_db->commit();
    delete m_db;
}

QStringList ContactIndexer::mimeTypes() const
{
    return QStringList() << KABC::Addressee::mimeType();
}

void ContactIndexer::index(const Akonadi::Item& item)
{
    KABC::Addressee addresse;
    try {
        addresse = item.payload<KABC::Addressee>();
    } catch (const Akonadi::PayloadException&) {
        return;
    }

    Xapian::Document doc;
    Xapian::TermGenerator termGen;
    termGen.set_database(*m_db);
    termGen.set_document(doc);

    QString name;
    if (!addresse.formattedName().isEmpty()) {
        name = addresse.formattedName();
    }
    else {
        name = addresse.assembledName();
    }

    std::string stdName = name.toStdString();
    std::string stdNick = addresse.nickName().toStdString();
    kDebug() << "Indexing" << name << addresse.nickName();

    termGen.index_text(stdName);
    termGen.index_text(stdNick);
    doc.add_boolean_term(addresse.uid().toStdString());

    termGen.index_text(stdName, 1, "NA");
    termGen.index_text(stdName, 1, "NI");

    Q_FOREACH (const QString& email, addresse.emails()) {
        std::string stdEmail = email.toStdString();

        doc.add_term(stdEmail);
        termGen.index_text(stdEmail);
    }

    m_db->replace_document(item.id(), doc);
    // TODO: Contact Groups?
}

void ContactIndexer::remove(const Akonadi::Item& item)
{
    try {
        m_db->delete_document(item.id());
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void ContactIndexer::commit()
{
    m_db->commit();
}

