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

#ifndef FILESEARCHSTORE_H
#define FILESEARCHSTORE_H

#include "searchstore.h"
#include <xapian.h>
#include <QSqlDatabase>

namespace Baloo {

class FileSearchStore : public SearchStore
{
public:
    FileSearchStore(QObject* parent, const QVariantList& args);
    virtual ~FileSearchStore();

    /**
     * Overwrite the default DB path. Generally used for testing
     */
    void setDbPath(const QString& path);

    virtual QStringList types();
    virtual int exec(const Query& query);
    virtual void close(int queryId);
    virtual bool next(int queryId);

    virtual Item::Id id(int queryId);
    virtual QUrl url(int queryId);

private:
    Xapian::Query toXapianQuery(const Term& term);
    Xapian::Query toXapianQuery(Xapian::Query::op op, const QList<Term>& terms);

    struct Iter {
        Xapian::MSet mset;
        Xapian::MSetIterator it;
    };

    QHash<int, Iter> m_queryMap;
    int m_nextId;

    QString m_dbPath;
    QSqlDatabase* m_sqlDb;
};

}
#endif // FILESEARCHSTORE_H
