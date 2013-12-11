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

#ifndef BALOO_XAPIANSEARCHSTORE_H
#define BALOO_XAPIANSEARCHSTORE_H

#include "searchstore.h"
#include "term.h"
#include <xapian.h>
#include <QMutex>

namespace Baloo {

/**
 * Implements a search store using Xapian
 */
class BALOO_CORE_EXPORT XapianSearchStore : public SearchStore
{
public:
    explicit XapianSearchStore(QObject* parent = 0);

    virtual int exec(const Query& query);
    virtual void close(int queryId);
    virtual bool next(int queryId);

    virtual Item::Id id(int queryId);
    virtual QUrl url(int queryId);

    /**
     * Set the path of the xapian database
     */
    virtual void setDbPath(const QString& path);
    virtual QString dbPath();

protected:
    /**
     * The derived class should implement the logic for constructing the appropriate
     * Xapian::Query class from the given values.
     */
    virtual Xapian::Query constructQuery(const QString& property,
                                         const QVariant& value,
                                         Term::Comparator com) = 0;

    virtual Xapian::Query constructFilterQuery(int year, int month, int day);

    /**
     * Apply any final touches to the query
     */
    virtual Xapian::Query finalizeQuery(const Xapian::Query& query);

    /**
     * Returns the url for the document with id \p docid.
     */
    virtual QUrl constructUrl(const Xapian::docid& docid) = 0;

    /**
     * Gives a list of types which have been provided with the query.
     * This must return the appropriate query which will be ANDed with
     * the final query
     */
    virtual Xapian::Query convertTypes(const QStringList& types) = 0;

    /**
     * The prefix that should be used when converting an integer
     * id to a byte array
     */
    virtual QByteArray idPrefix() = 0;

    Xapian::Document docForQuery(int queryId);

    /**
     * Convenience function to AND two Xapian queries together.
     */
    Xapian::Query andQuery(const Xapian::Query& a, const Xapian::Query& b);

    Xapian::Database* xapianDb();

private:
    Xapian::Query toXapianQuery(const Term& term);
    Xapian::Query toXapianQuery(Xapian::Query::op op, const QList<Term>& terms);

    struct Result {
        Xapian::MSet mset;
        Xapian::MSetIterator it;

        uint lastId;
        QUrl lastUrl;
    };

    QHash<int, Result> m_queryMap;
    int m_nextId;

    QString m_dbPath;
    QMutex m_mutex;

    Xapian::Database* m_db;
};

}

#endif // BALOO_XAPIANSEARCHSTORE_H
