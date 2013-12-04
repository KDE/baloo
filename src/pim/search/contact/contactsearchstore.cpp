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

#include "contactsearchstore.h"
#include "item.h"
#include "term.h"
#include "query.h"

#include <QVector>

#include <KStandardDirs>
#include <KDebug>
#include <KUrl>

using namespace Baloo;

ContactSearchStore::ContactSearchStore(QObject* parent, const QVariantList&)
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

QString ContactSearchStore::prefix(const QString& property)
{
    return m_prefix.value(property.toLower());
}

QUrl ContactSearchStore::urlFromDoc(const Xapian::docid& docid)
{
    KUrl url;
    url.setProtocol(QLatin1String("akonadi"));
    url.addQueryItem(QLatin1String("item"), QString::number(docid));

    return url;
}

BALOO_EXPORT_SEARCHSTORE(Baloo::ContactSearchStore, "baloo_contactsearchstore")
