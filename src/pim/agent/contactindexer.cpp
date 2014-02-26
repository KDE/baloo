/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "contactindexer.h"

#include <KABC/Addressee>
#include <Akonadi/Collection>

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
    else if (!addresse.assembledName().isEmpty()) {
        name = addresse.assembledName();
    }
    else {
        name = addresse.name();
    }

    std::string stdName = name.toStdString();
    std::string stdNick = addresse.nickName().toStdString();
    kDebug() << "Indexing" << name << addresse.nickName();

    termGen.index_text(stdName);
    termGen.index_text(stdNick);
    doc.add_boolean_term(addresse.uid().toStdString());

    termGen.index_text(stdName, 1, "NA");
    termGen.index_text(stdNick, 1, "NI");

    Q_FOREACH (const QString& email, addresse.emails()) {
        std::string stdEmail = email.toStdString();

        doc.add_term(stdEmail);
        termGen.index_text(stdEmail);
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Baloo::ContactIndexer::index",
               "Item does not have a valid parent collection");

    Akonadi::Entity::Id colId = item.parentCollection().id();
    QByteArray term = 'C' + QByteArray::number(colId);
    doc.add_boolean_term(term.data());

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

void ContactIndexer::remove(const Akonadi::Collection& collection)
{
    try {
        Xapian::Query query('C'+ QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void ContactIndexer::commit()
{
    m_db->commit();
}

