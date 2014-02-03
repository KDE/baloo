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

#ifndef QUERY_H
#define QUERY_H

#include "core_export.h"
#include "resultiterator.h"

#include <QVariant>

namespace Baloo {

class Term;

class BALOO_CORE_EXPORT Query
{
public:
    Query();
    Query(const Term& t);
    Query(const Query& rhs);
    ~Query();

    void setTerm(const Term& t);
    Term term() const;

    /**
     * Add a type to the results of the query.
     *
     * Each Item in the result must contain one of the types.
     * This is generally used to filter only Files, Emails, Tags, etc
     *
     * One can add multiple types in one go by separating individual types
     * with a '/'. Eg - "File/Audio"
     */
    void addType(const QString& type);
    void addTypes(const QStringList& typeList);
    void setType(const QString& type);
    void setTypes(const QStringList& types);

    QStringList types() const;

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

    void setOffset(uint offset);
    uint offset() const;

    /**
     * Filter the results in the specified date range.
     *
     * The year/month/day may be set to -1 in order to ignore it.
     */
    void setDateFilter(int year, int month = -1, int day = -1);

    int yearFilter() const;
    int monthFilter() const;
    int dayFilter() const;

    // FIXME: Sorting?

    /**
     * Adds a custom option which any search backend could use
     * to configure the query result.
     *
     * Each backend has their own custom options which should be
     * looked up in their corresponding documentation
     */
    void addCustomOption(const QString& option, const QVariant& value);
    void removeCustomOption(const QString& option);
    QVariant customOption(const QString& option) const;
    QVariantHash customOptions() const;

    ResultIterator exec();

    QByteArray toJSON();
    static Query fromJSON(const QByteArray& arr);

    QUrl toSearchUrl(const QString& title = QString());
    static Query fromSearchUrl(const QUrl& url);

    bool operator == (const Query& rhs) const;

    Query& operator=(const Query& rhs);

private:
    class Private;
    Private* d;
};

}
#endif // QUERY_H
