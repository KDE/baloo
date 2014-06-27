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

#include "contactsearchstore.h"

#include <QDebug>

using namespace Baloo;

ContactSearchStore::ContactSearchStore(QObject* parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QLatin1String("name"), QLatin1String("NA"));
    m_prefix.insert(QLatin1String("nick"), QLatin1String("NI"));
    m_prefix.insert(QLatin1String("email"), QLatin1String("")); // Email currently doesn't map to anything
    m_prefix.insert(QLatin1String("collection"), QLatin1String("C"));


    m_valueProperties.insert(QLatin1String("birthday"), 0);
    m_valueProperties.insert(QLatin1String("anniversary"), 1);

    setDbPath(findDatabase(QLatin1String("contacts")));
}

QStringList ContactSearchStore::types()
{
    return QStringList() << QLatin1String("Akonadi") << QLatin1String("Contact");
}

#include "contactsearchstore.moc"
