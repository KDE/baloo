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

#include "../core/relation.h"

class Term;

class Query
{
public:
    Query();
    Query(const Term& t);

    /**
     * A term is added to the list of terms that are ANDed
     * in the query.
     *
     * Each of these terms must match in the query
     */
    void addTerm(const Term& t);
    void setTerms(const QList<Term>& terms);

    QList<Term> terms();

    /**
     * Add a type to the results of the query.
     *
     * Each Item in the result must contain one of the types.
     * This is generally used to filter only Files, Emails, Tags, etc
     */
    void addType(const QString& type);
    void setTypes(const QList<QString>& types);

    QList<QString> types();

    /**
     * Every Item in the result must contain the relation \p rel
     */
    void addRelation(const Relation& rel);
    void setRelations(const QList<Relation>& rel);

    QList<Relation> relations();

    // FIXME: Is this something we want?
    void add(const Term& term);
    void add(const QString& type);
    void add(const Relation& rel);

    /**
     * Set some text which should be used to search for Items. This
     * contain a single word or an entire sentence.
     *
     * Each search backend will interpret it in its own way, and try
     * to give the best possible results.
     */
    void setSearchString(const QString& str);
    QString searchString();

    /**
     * Only a maximum of \p limit results will be returned
     */
    void setLimit(int limit);
    int limit();

    // FIXME: Sorting?
};

#endif // QUERY_H
