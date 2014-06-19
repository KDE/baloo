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
#include "xapiandocument.h"

#include <KABC/Addressee>
#include <KABC/ContactGroup>
#include <Akonadi/Collection>

ContactIndexer::ContactIndexer(const QString& path):
    AbstractIndexer(), m_db( 0 )
{
    try {
        m_db = new Baloo::XapianDatabase(path, true);
    }
    catch (const Xapian::DatabaseCorruptError& err) {
        kError() << "Database Corrupted - What did you do?";
        kError() << err.get_error_string();
        m_db = 0;
    }
    catch (const Xapian::Error &e) {
        kError() << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = 0;
    }
}

ContactIndexer::~ContactIndexer()
{
    if (m_db) {
        m_db->commit();
        delete m_db;
    }
}

QStringList ContactIndexer::mimeTypes() const
{
    return QStringList() << KABC::Addressee::mimeType() << KABC::ContactGroup::mimeType();
}

bool ContactIndexer::indexContact(const Akonadi::Item& item)
{
    if (!m_db)
        return false;
    KABC::Addressee addresse;
    try {
        addresse = item.payload<KABC::Addressee>();
    } catch (const Akonadi::PayloadException&) {
        return false;
    }

    Baloo::XapianDocument doc;

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

    kDebug() << "Indexing" << name << addresse.nickName();

    doc.indexText(name);
    doc.indexText(addresse.nickName());
    doc.indexText(addresse.uid());

    doc.indexText(name, QLatin1String("NA"));
    doc.indexText(addresse.nickName(), QLatin1String("NI"));

    Q_FOREACH (const QString& email, addresse.emails()) {
        doc.addTerm(email);
        doc.indexText(email);
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Baloo::ContactIndexer::index",
               "Item does not have a valid parent collection");

    const Akonadi::Entity::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, QLatin1String("C"));

    if (addresse.birthday().isValid()) {
        const QString julianDay = QString::number(addresse.birthday().date().toJulianDay());
        doc.addValue(0, julianDay);
    }
    //TODO index anniversary ?

    m_db->replaceDocument(item.id(), doc);
    return true;
}

void ContactIndexer::indexContactGroup(const Akonadi::Item& item)
{
    if (!m_db)
        return;
    KABC::ContactGroup group;
    try {
        group = item.payload<KABC::ContactGroup>();
    } catch (const Akonadi::PayloadException&) {
        return;
    }

    Baloo::XapianDocument doc;

    const QString name = group.name();
    doc.indexText(name);
    doc.indexText(name, QLatin1String("NA"));


    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Baloo::ContactIndexer::index",
               "Item does not have a valid parent collection");

    const Akonadi::Entity::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, QLatin1String("C"));
    m_db->replaceDocument(item.id(), doc);
}


void ContactIndexer::index(const Akonadi::Item& item)
{
    if (!indexContact(item)) {
        indexContactGroup(item);
    }
}

void ContactIndexer::remove(const Akonadi::Item& item)
{
    if (m_db)
        m_db->deleteDocument(item.id());
}

void ContactIndexer::remove(const Akonadi::Collection& collection)
{
    if (!m_db)
        return;
    try {
        Xapian::Database* db = m_db->db();
        Xapian::Query query('C'+ QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, db->get_doccount());
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
    if (!m_db)
        m_db->commit();
}

void ContactIndexer::move(const Akonadi::Item::Id& itemId,
                        const Akonadi::Entity::Id& from,
                        const Akonadi::Entity::Id& to)
{
    if (!m_db)
        return;
    Baloo::XapianDocument doc;
    try {
        doc = m_db->document(itemId);
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.removeTermStartsWith(ft.data());
    doc.addBoolTerm(QString::fromLatin1(tt.data()));
    m_db->replaceDocument(doc.doc().get_docid(), doc);
}

