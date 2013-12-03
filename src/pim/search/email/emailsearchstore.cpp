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

    // Collection?
    // Flags?

    const QString path = KStandardDirs::locateLocal("data", "baloo/email/");
    setDbPath(path);
}

QStringList EmailSearchStore::types()
{
    return QStringList() << "Akonadi" << "Email";
}

QString EmailSearchStore::prefix(const QString& property)
{
    return m_prefix.value(property.toLower());
}

QUrl EmailSearchStore::urlFromDoc(const Xapian::docid& docid)
{
    KUrl url;
    url.setProtocol(QLatin1String("akonadi"));
    url.addQueryItem(QLatin1String("item"), QString::number(docid));

    return url;
}

BALOO_EXPORT_SEARCHSTORE(Baloo::EmailSearchStore, "baloo_emailsearchstore")
