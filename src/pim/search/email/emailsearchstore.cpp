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

#include "emailsearchstore.h"
#include "item.h"
#include "term.h"
#include "query.h"

#include <QVector>

#include <KStandardDirs>
#include <KDebug>
#include <KUrl>

using namespace Baloo;

EmailSearchStore::EmailSearchStore(QObject* parent, const QVariantList&)
    : XapianSearchStore(parent)
{
    m_prefix.insert("from", "F");
    m_prefix.insert("to", "T");
    m_prefix.insert("cc", "CC");
    m_prefix.insert("bcc", "BC");
    m_prefix.insert("subject", "S");
    m_prefix.insert("collection", "C");

    // TODO: Add body flag?
    // TODO: Add tags?

    /*
    // TODO: Add these when there is support for them in the indexer
    m_prefix.insert("reply-to", "RT");
    m_prefix.insert("list-id", "LI");
    m_prefix.insert("resent-from", "RF");
    m_prefix.insert("x-loop", "XL");
    m_prefix.insert("x-mailing-list", "XML");
    m_prefix.insert("x-spam-flag", "XSF");
    m_prefix.insert("organization", "O");
    */

    // Boolean Flags
    m_prefix.insert("isimportant", "I");
    m_prefix.insert("istoact", "");
    m_prefix.insert("iswatched", "W");
    m_prefix.insert("isdeleted", "");
    m_prefix.insert("isspam", "");
    m_prefix.insert("isreplied", "");
    m_prefix.insert("isignored", "");
    m_prefix.insert("isforwarded", "");
    m_prefix.insert("issent", "");
    m_prefix.insert("isqueued", "");
    m_prefix.insert("isham", "");
    m_prefix.insert("isread", "R");

    m_boolProperties << "isimportant" << "istoAct" << "iswatched" << "isdeleted" << "isspam"
                     << "isreplied" << "isignored" << "isforwarded" << "issent" << "isqueued"
                     << "isham" << "isread";

    // TODO: Sent date? Age in days?

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
    if (value.isNull())
        return Xapian::Query();

    QString prop = property.toLower();
    if (m_boolProperties.contains(prop)) {
        QString p = m_prefix.value(prop);
        if (p.isEmpty())
            return Xapian::Query();

        std::string term("B");
        bool isTrue = false;

        if (value.isNull() )
            isTrue = true;

        if (value.type() == QVariant::Bool) {
            isTrue = value.toBool();
        }

        if (isTrue)
            term += p.toStdString();
        else
            term += 'N' + p.toStdString();

        return Xapian::Query(term);
    }

    if (com == Term::Contains) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p = m_prefix.value(property.toLower()).toStdString();
        if (p.empty())
            return Xapian::Query();

        std::string str = value.toString().toStdString();
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        return parser.parse_query(str, flags, p);
    }

    return Xapian::Query(value.toString().toStdString());
}

QUrl EmailSearchStore::constructUrl(const Xapian::docid& docid)
{
    KUrl url;
    url.setProtocol(QLatin1String("akonadi"));
    url.addQueryItem(QLatin1String("item"), QString::number(docid));

    return url;
}

QString EmailSearchStore::text(int queryId)
{
    std::string data = docForQuery(queryId).get_data();

    QString subject = QString::fromStdString(data);
    if (subject.isEmpty())
        return QLatin1String("No Subject");

    return subject;
}

BALOO_EXPORT_SEARCHSTORE(Baloo::EmailSearchStore, "baloo_emailsearchstore")
