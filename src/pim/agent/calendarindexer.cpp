/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014 Laurent Montel <montel@kde.org>
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

#include "calendarindexer.h"

#include <QTextDocument>


CalendarIndexer::CalendarIndexer(const QString& path)
    : AbstractIndexer(), m_termGen( 0 )
{
    m_db = new Xapian::WritableDatabase(path.toUtf8().constData(), Xapian::DB_CREATE_OR_OPEN);
}

CalendarIndexer::~CalendarIndexer()
{
    m_db->commit();
    delete m_db;
}

QStringList CalendarIndexer::mimeTypes() const
{
    return QStringList() << QString::fromLatin1("application/x-vnd.akonadi.calendar.event")
                         << QString::fromLatin1("application/x-vnd.akonadi.calendar.todo")
                         << QString::fromLatin1("application/x-vnd.akonadi.calendar.journal")
                         << QString::fromLatin1("application/x-vnd.akonadi.calendar.freebusy");
}

void CalendarIndexer::index(const Akonadi::Item &item)
{
}

void CalendarIndexer::commit()
{
    m_db->commit();
}

void CalendarIndexer::remove(const Akonadi::Item &item)
{
    try {
        m_db->delete_document(item.id());
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void CalendarIndexer::remove(const Akonadi::Collection& collection)
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

void CalendarIndexer::move(const Akonadi::Item::Id& itemId,
                        const Akonadi::Entity::Id& from,
                        const Akonadi::Entity::Id& to)
{
    Xapian::Document doc;
    try {
        doc = m_db->get_document(itemId);
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.remove_term(ft.data());
    doc.add_boolean_term(tt.data());
    m_db->replace_document(doc.get_docid(), doc);
}

