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

#include "emailsearchstore.h"
#include "term.h"
#include "query.h"
#include "agepostingsource.h"

#include <KStandardDirs>
#include <KDebug>

using namespace Baloo;

EmailSearchStore::EmailSearchStore(QObject* parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert("from", "F");
    m_prefix.insert("to", "T");
    m_prefix.insert("cc", "CC");
    m_prefix.insert("bcc", "BC");
    m_prefix.insert("subject", "S");
    m_prefix.insert("collection", "C");
    m_prefix.insert("replyto", "RT");
    m_prefix.insert("organization", "O");
    m_prefix.insert("listid", "LI");
    m_prefix.insert("resentfrom", "RF");
    m_prefix.insert("xloop", "XL");
    m_prefix.insert("xmailinglist", "XML");
    m_prefix.insert("xspamflag", "XSF");

    m_prefix.insert("body", "BO");
    m_prefix.insert("headers", "H");

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    m_prefix.insert("isimportant", "I");
    m_prefix.insert("istoact", "T");
    m_prefix.insert("iswatched", "W");
    m_prefix.insert("isdeleted", "D");
    m_prefix.insert("isspam", "S");
    m_prefix.insert("isreplied", "E");
    m_prefix.insert("isignored", "G");
    m_prefix.insert("isforwarded", "F");
    m_prefix.insert("issent", "N");
    m_prefix.insert("isqueued", "Q");
    m_prefix.insert("isham", "H");
    m_prefix.insert("isread", "R");
    m_prefix.insert("hasattachment", "A");
    m_prefix.insert("isencrypted", "C");

    m_boolProperties << "isimportant" << "istoact" << "iswatched" << "isdeleted" << "isspam"
                     << "isreplied" << "isignored" << "isforwarded" << "issent" << "isqueued"
                     << "isham" << "isread" << "hasattachment" << "isencrypted";

    m_valueProperties.insert("date", 0);
    m_valueProperties.insert("size", 1);
    m_valueProperties.insert("onlydate", 2);

    const QString path = KStandardDirs::locateLocal("data", "baloo/email/");
    setDbPath(path);
}

QStringList EmailSearchStore::types()
{
    return QStringList() << "Akonadi" << "Email";
}

Xapian::Query EmailSearchStore::constructQuery(const QString& property, const QVariant& value,
                                               Term::Comparator com)
{
    //TODO is this special case necessary? maybe we can also move it to PIM
    if (com == Term::Contains) {
        if (!m_prefix.contains(property.toLower())) {
            return Xapian::Query();
        }
    }
    return PIMSearchStore::constructQuery(property, value, com);
}

QString EmailSearchStore::text(int queryId)
{
    std::string data = docForQuery(queryId).get_data();

    QString subject = QString::fromUtf8(data.c_str(), data.length());
    if (subject.isEmpty())
        return QLatin1String("No Subject");

    return subject;
}

Xapian::Query EmailSearchStore::finalizeQuery(const Xapian::Query& query)
{
    AgePostingSource ps(0);
    return Xapian::Query(Xapian::Query::OP_AND_MAYBE, query, Xapian::Query(&ps));
}

BALOO_EXPORT_SEARCHSTORE(Baloo::EmailSearchStore, "baloo_emailsearchstore")
