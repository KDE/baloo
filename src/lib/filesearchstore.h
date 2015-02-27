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

#include "lucenesearchstore.h"

namespace Baloo {

class FileSearchStore : public LuceneSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Baloo::SearchStore)

public:
    FileSearchStore(QObject* parent = 0);
    virtual ~FileSearchStore();

    virtual void setIndexPath(const QString& path);
    virtual QStringList types();

protected:
    virtual Lucene::QueryPtr constructQuery(const QString& property,
                                         const QVariant& value,
                                         Term::Comparator com);

    virtual Lucene::QueryPtr constructFilterQuery(int year, int month, int day);
    virtual Lucene::QueryPtr applyIncludeFolder(const Lucene::QueryPtr& q, QString includeFolder);

    virtual Lucene::QueryPtr convertTypes(const QStringList& types);
    virtual QString constructFilePath(int docid);

    virtual QByteArray idPrefix() { return QByteArray("file"); }

private:
    QHash<QString, QString> m_prefixes;

    QString fetchPrefix(const QString& property) const;
};

}
#endif // FILESEARCHSTORE_H
