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

#ifndef CALENDARINDEXER_H
#define CALENDARINDEXER_H

#include "abstractindexer.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <xapian.h>

class CalendarIndexer : public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    CalendarIndexer(const QString& path);
    ~CalendarIndexer();

    QStringList mimeTypes() const;

    void index(const Akonadi::Item &item);
    void commit();

    void remove(const Akonadi::Item &item);
    void remove(const Akonadi::Collection &collection);
    void move(const Akonadi::Item::Id &itemId, const Akonadi::Entity::Id &from, const Akonadi::Entity::Id &to);
private:
    Xapian::WritableDatabase *m_db;
    Xapian::Document *m_doc;
    Xapian::TermGenerator *m_termGen;
};

#endif // CALENDARINDEXER_H
