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

#ifndef QUERY_H
#define QUERY_H

#include "relation.h"
#include "resultiterator.h"

namespace Baloo {

class Term;

class BALOO_CORE_EXPORT Query
{
public:
    Query();
    Query(const Term& t);

    void setTerm(const Term& t);
    Term term() const;

    /**
     * Add a type to the results of the query.
     *
     * Each Item in the result must contain one of the types.
     * This is generally used to filter only Files, Emails, Tags, etc
     */
    void addType(const QString& type);
    void setTypes(const QStringList& types);

    QStringList types() const;

    /**
     * Every Item in the result must contain the relation \p rel
     */
    void addRelation(const Relation& rel);
    void setRelations(const QList<Relation>& rel);

    QList<Relation> relations() const;

    /**
     * Set some text which should be used to search for Items. This
     * contain a single word or an entire sentence.
     *
     * Each search backend will interpret it in its own way, and try
     * to give the best possible results.
     */
    void setSearchString(const QString& str);
    QString searchString() const;

    /**
     * Only a maximum of \p limit results will be returned.
     * By default the limit is 100000.
     */
    void setLimit(uint limit);
    uint limit() const;

    // FIXME: Sorting?

    /**
     * Adds a custom option which any search backend could use
     * to configure the query result.
     *
     * Each backend has their own custom options which should be
     * looked up in their corresponding documentation
     */
    // TODO:
    //void addCustomOption(const QString& option, const QString& value);
    //void removeCustomOption(const QString& option);
    //QString customOption(const QString& option);

    ResultIterator exec();

private:
    class Private;
    Private* d;
};

}
#endif // QUERY_H
