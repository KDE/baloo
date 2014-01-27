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
#include "term.h"
#include "query.h"


#include <KStandardDirs>
#include <KDebug>
#include <KUrl>

using namespace Baloo;

ContactSearchStore::ContactSearchStore(QObject* parent)
    : XapianSearchStore(parent)
{
    m_prefix.insert("name", "NA");
    m_prefix.insert("nick", "NI");
    m_prefix.insert("email", ""); // Email currently doesn't map to anything

    const QString path = KStandardDirs::locateLocal("data", "baloo/contacts/");
    setDbPath(path);
}

QStringList ContactSearchStore::types()
{
    return QStringList() << "Akonadi" << "Contact";
}

Xapian::Query ContactSearchStore::constructQuery(const QString& property, const QVariant& value,
                                                 Term::Comparator com)
{
    if (value.isNull())
        return Xapian::Query();

    if (com == Term::Contains) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p = m_prefix.value(property.toLower()).toStdString();
        std::string str = value.toString().toStdString();
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        return parser.parse_query(str, flags, p);
    }

    return Xapian::Query(value.toString().toStdString());
}


QUrl ContactSearchStore::constructUrl(const Xapian::docid& docid)
{
    KUrl url;
    url.setProtocol(QLatin1String("akonadi"));
    url.addQueryItem(QLatin1String("item"), QString::number(docid));

    return url;
}

BALOO_EXPORT_SEARCHSTORE(Baloo::ContactSearchStore, "baloo_contactsearchstore")
