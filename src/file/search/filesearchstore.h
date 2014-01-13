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

#ifndef FILESEARCHSTORE_H
#define FILESEARCHSTORE_H

#include "xapiansearchstore.h"
#include <QSqlDatabase>

#include <QMutex>

namespace Baloo {

class FileSearchStore : public XapianSearchStore
{
public:
    FileSearchStore(QObject* parent, const QVariantList& args);
    virtual ~FileSearchStore();

    virtual void setDbPath(const QString& path);
    virtual QStringList types();
    virtual QString text(int queryId);

protected:
    virtual Xapian::Query constructQuery(const QString& property,
                                         const QVariant& value,
                                         Term::Comparator com);

    virtual Xapian::Query constructFilterQuery(int year, int month, int day);
    virtual Xapian::Query applyCustomOptions(const Xapian::Query& q, const QVariantHash& options);

    virtual Xapian::Query convertTypes(const QStringList& types);
    virtual QUrl constructUrl(const Xapian::docid& docid);

    virtual QByteArray idPrefix() { return QByteArray("file"); }

private:
    QSqlDatabase* m_sqlDb;
    QMutex m_sqlMutex;

    QHash<QString, std::string> m_prefixes;
};

}
#endif // FILESEARCHSTORE_H
