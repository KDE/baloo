/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#include "searchstore.h"
#include "query.h"

#include "database.h"
#include "enginequery.h"
#include "queryparser.h"

#include <QStandardPaths>

#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

SearchStore::SearchStore()
    : m_db(0)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
    m_db = new Database(path);
    m_db->open();
    m_db->transaction(Database::ReadOnly);

    m_prefixes.insert(QByteArray("filename"), QByteArray("F"));
    m_prefixes.insert(QByteArray("mimetype"), QByteArray("M"));
    m_prefixes.insert(QByteArray("rating"), QByteArray("R"));
    m_prefixes.insert(QByteArray("tag"), QByteArray("TA"));
    m_prefixes.insert(QByteArray("tags"), QByteArray("TA"));
    m_prefixes.insert(QByteArray("usercomment"), QByteArray("C"));
    m_prefixes.insert(QByteArray("type"), QByteArray("T"));
    m_prefixes.insert(QByteArray("kind"), QByteArray("T"));
}

SearchStore::~SearchStore()
{
    delete m_db;
}

QStringList SearchStore::types()
{
    return QStringList() << QLatin1String("Audio") << QLatin1String("Video") << QLatin1String("Document")
                         << QLatin1String("Image") << QLatin1String("Archive") << QLatin1String("Folder");
}

QVector<quint64> SearchStore::exec(const Query& query)
{
    if (!query.searchString().isEmpty()) {
        QueryParser parser;
        EngineQuery eq = parser.parseQuery(query.searchString());
        return m_db->exec(eq);
    }

    return QVector<quint64>();
}

QString SearchStore::filePath(quint64 id)
{
    Q_ASSERT(id > 0);
    return m_db->documentUrl(id);
}

QByteArray SearchStore::fetchPrefix(const QByteArray& property) const
{
    auto it = m_prefixes.constFind(property.toLower());
    if (it != m_prefixes.constEnd()) {
        return it.value();
    }
    else {
        KFileMetaData::PropertyInfo pi = KFileMetaData::PropertyInfo::fromName(property);
        if (pi.property() == KFileMetaData::Property::Empty) {
            qDebug() << "Property" << property << "not found";
            return QByteArray();
        }
        int propPrefix = static_cast<int>(pi.property());
        return 'X' + QByteArray::number(propPrefix);
    }

}
