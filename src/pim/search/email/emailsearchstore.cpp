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
    m_prefix.insert(QLatin1String("from"), QLatin1String("F"));
    m_prefix.insert(QLatin1String("to"), QLatin1String("T"));
    m_prefix.insert(QLatin1String("cc"), QLatin1String("CC"));
    m_prefix.insert(QLatin1String("bcc"), QLatin1String("BC"));
    m_prefix.insert(QLatin1String("subject"), QLatin1String("SU"));
    m_prefix.insert(QLatin1String("collection"), QLatin1String("C"));
    m_prefix.insert(QLatin1String("replyto"), QLatin1String("RT"));
    m_prefix.insert(QLatin1String("organization"), QLatin1String("O"));
    m_prefix.insert(QLatin1String("listid"), QLatin1String("LI"));
    m_prefix.insert(QLatin1String("resentfrom"), QLatin1String("RF"));
    m_prefix.insert(QLatin1String("xloop"), QLatin1String("XL"));
    m_prefix.insert(QLatin1String("xmailinglist"), QLatin1String("XML"));
    m_prefix.insert(QLatin1String("xspamflag"), QLatin1String("XSF"));

    m_prefix.insert(QLatin1String("body"), QLatin1String("BO"));
    m_prefix.insert(QLatin1String("headers"), QLatin1String("HE"));

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    m_prefix.insert(QLatin1String("isimportant"), QLatin1String("I"));
    m_prefix.insert(QLatin1String("istoact"), QLatin1String("T"));
    m_prefix.insert(QLatin1String("iswatched"), QLatin1String("W"));
    m_prefix.insert(QLatin1String("isdeleted"), QLatin1String("D"));
    m_prefix.insert(QLatin1String("isspam"), QLatin1String("S"));
    m_prefix.insert(QLatin1String("isreplied"), QLatin1String("E"));
    m_prefix.insert(QLatin1String("isignored"), QLatin1String("G"));
    m_prefix.insert(QLatin1String("isforwarded"), QLatin1String("F"));
    m_prefix.insert(QLatin1String("issent"), QLatin1String("N"));
    m_prefix.insert(QLatin1String("isqueued"), QLatin1String("Q"));
    m_prefix.insert(QLatin1String("isham"), QLatin1String("H"));
    m_prefix.insert(QLatin1String("isread"), QLatin1String("R"));
    m_prefix.insert(QLatin1String("hasattachment"), QLatin1String("A"));
    m_prefix.insert(QLatin1String("isencrypted"), QLatin1String("C"));
    m_prefix.insert(QLatin1String("hasinvitation"), QLatin1String("V"));

    m_boolProperties << QLatin1String("isimportant") << QLatin1String("istoact") << QLatin1String("iswatched") << QLatin1String("isdeleted") << QLatin1String("isspam")
                     << QLatin1String("isreplied") << QLatin1String("isignored") << QLatin1String("isforwarded") << QLatin1String("issent") << QLatin1String("isqueued")
                     << QLatin1String("isham") << QLatin1String("isread") << QLatin1String("hasattachment") << QLatin1String("isencrypted") << QLatin1String("hasinvitation");

    m_valueProperties.insert(QLatin1String("date"), 0);
    m_valueProperties.insert(QLatin1String("size"), 1);
    m_valueProperties.insert(QLatin1String("onlydate"), 2);

    setDbPath(findDatabase(QLatin1String("email")));
}

QStringList EmailSearchStore::types()
{
    return QStringList() << QLatin1String("Akonadi") << QLatin1String("Email");
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
    Xapian::Document doc = docForQuery(queryId);
    std::string data;
    try {
        data = doc.get_data();
    }
    catch (const Xapian::Error&) {
        // Nothing to do, move along
    }

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
