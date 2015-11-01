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

#include "query.h"
#include "term.h"
#include "advancedqueryparser.h"
#include "searchstore.h"

#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QList>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>

using namespace Baloo;

const int defaultLimit = -1;

class Baloo::Query::Private {
public:
    Private() {
        m_limit = defaultLimit;
        m_offset = 0;
        m_yearFilter = 0;
        m_monthFilter = 0;
        m_dayFilter = 0;
        m_sortingOption = SortAuto;
    }
    Term m_term;

    QStringList m_types;
    QString m_searchString;
    int m_limit;
    uint m_offset;

    int m_yearFilter;
    int m_monthFilter;
    int m_dayFilter;

    SortingOption m_sortingOption;
    QString m_includeFolder;
};

Query::Query()
    : d(new Private)
{
}

Query::Query(const Query& rhs)
    : d(new Private(*rhs.d))
{
}

Query::~Query()
{
    delete d;
}

void Query::addType(const QString& type)
{
    d->m_types << type.split(QLatin1Char('/'), QString::SkipEmptyParts);
}

void Query::addTypes(const QStringList& typeList)
{
    Q_FOREACH (const QString& type, typeList) {
        addType(type);
    }
}

void Query::setType(const QString& type)
{
    d->m_types.clear();
    addType(type);
}

void Query::setTypes(const QStringList& types)
{
    d->m_types = types;
}

QStringList Query::types() const
{
    return d->m_types;
}

QString Query::searchString() const
{
    return d->m_searchString;
}

void Query::setSearchString(const QString& str)
{
    d->m_searchString = str;

    AdvancedQueryParser parser;
    d->m_term = parser.parse(str);
}

uint Query::limit() const
{
    return d->m_limit;
}

void Query::setLimit(uint limit)
{
    d->m_limit = limit;
}

uint Query::offset() const
{
    return d->m_offset;
}

void Query::setOffset(uint offset)
{
    d->m_offset = offset;
}

void Query::setDateFilter(int year, int month, int day)
{
    d->m_yearFilter = year;
    d->m_monthFilter = month;
    d->m_dayFilter = day;
}

int Query::yearFilter() const
{
    return d->m_yearFilter;
}

int Query::monthFilter() const
{
    return d->m_monthFilter;
}

int Query::dayFilter() const
{
    return d->m_dayFilter;
}

void Query::setSortingOption(Query::SortingOption option)
{
    d->m_sortingOption = option;
}

Query::SortingOption Query::sortingOption() const
{
    return d->m_sortingOption;
}

QString Query::includeFolder() const
{
    return d->m_includeFolder;
}

void Query::setIncludeFolder(const QString& folder)
{
    d->m_includeFolder = folder;
}

ResultIterator Query::exec()
{
    Term term(d->m_term);
    if (!d->m_types.isEmpty()) {
        for (const QString& type : d->m_types) {
            term = term && Term(QStringLiteral("type"), type);
        }
    }

    if (!d->m_includeFolder.isEmpty()) {
        term = term && Term(QStringLiteral("includefolder"), d->m_includeFolder);
    }

    if (d->m_yearFilter || d->m_monthFilter || d->m_dayFilter) {
        QByteArray ba = QByteArray::number(d->m_yearFilter);
        if (d->m_monthFilter < 10)
            ba += '0';
        ba += QByteArray::number(d->m_monthFilter);
        if (d->m_dayFilter < 10)
            ba += '0';
        ba += QByteArray::number(d->m_dayFilter);

        term = term && Term(QStringLiteral("modified"), ba, Term::Equal);
    }

    SearchStore searchStore;
    QStringList result = searchStore.exec(term, d->m_offset, d->m_limit, d->m_sortingOption == SortAuto);
    return ResultIterator(result);
}

QByteArray Query::toJSON()
{
    QVariantMap map;

    if (!d->m_types.isEmpty())
        map[QStringLiteral("type")] = d->m_types;

    if (d->m_limit != defaultLimit)
        map[QStringLiteral("limit")] = d->m_limit;

    if (d->m_offset)
        map[QStringLiteral("offset")] = d->m_offset;

    if (!d->m_searchString.isEmpty())
        map[QStringLiteral("searchString")] = d->m_searchString;

    if (d->m_term.isValid())
        map[QStringLiteral("term")] = QVariant(d->m_term.toVariantMap());

    if (d->m_yearFilter >= 0)
        map[QStringLiteral("yearFilter")] = d->m_yearFilter;
    if (d->m_monthFilter >= 0)
        map[QStringLiteral("monthFilter")] = d->m_monthFilter;
    if (d->m_dayFilter >= 0)
        map[QStringLiteral("dayFilter")] = d->m_dayFilter;

    if (d->m_sortingOption != SortAuto)
        map[QStringLiteral("sortingOption")] = static_cast<int>(d->m_sortingOption);

    if (!d->m_includeFolder.isEmpty())
        map[QStringLiteral("includeFolder")] = d->m_includeFolder;

    QJsonObject jo = QJsonObject::fromVariantMap(map);
    QJsonDocument jdoc;
    jdoc.setObject(jo);
    return jdoc.toJson();
}

// static
Query Query::fromJSON(const QByteArray& arr)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(arr);
    const QVariantMap map = jdoc.object().toVariantMap();

    Query query;
    query.d->m_types = map[QStringLiteral("type")].toStringList();

    if (map.contains(QStringLiteral("limit")))
        query.d->m_limit = map[QStringLiteral("limit")].toUInt();
    else
        query.d->m_limit = defaultLimit;

    query.d->m_offset = map[QStringLiteral("offset")].toUInt();
    query.d->m_searchString = map[QStringLiteral("searchString")].toString();
    query.d->m_term = Term::fromVariantMap(map[QStringLiteral("term")].toMap());

    if (map.contains(QStringLiteral("yearFilter")))
        query.d->m_yearFilter = map[QStringLiteral("yearFilter")].toInt();
    if (map.contains(QStringLiteral("monthFilter")))
        query.d->m_monthFilter = map[QStringLiteral("monthFilter")].toInt();
    if (map.contains(QStringLiteral("dayFilter")))
        query.d->m_dayFilter = map[QStringLiteral("dayFilter")].toInt();

    if (map.contains(QStringLiteral("sortingOption"))) {
        int option = map.value(QStringLiteral("sortingOption")).toInt();
        query.d->m_sortingOption = static_cast<SortingOption>(option);
    }


    if (map.contains(QStringLiteral("includeFolder"))) {
        query.d->m_includeFolder = map.value(QStringLiteral("includeFolder")).toString();
    }

    return query;
}

QUrl Query::toSearchUrl(const QString& title)
{
    QUrl url;
    url.setScheme(QStringLiteral("baloosearch"));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("json"), QString::fromUtf8(toJSON()));

    if (title.size())
        urlQuery.addQueryItem(QStringLiteral("title"), title);

    url.setQuery(urlQuery);
    return url;
}

Query Query::fromSearchUrl(const QUrl& url)
{
    if (url.scheme() != QLatin1String("baloosearch"))
        return Query();

    QUrlQuery urlQuery(url);
    QString jsonString = urlQuery.queryItemValue(QStringLiteral("json"), QUrl::FullyDecoded);
    return Query::fromJSON(jsonString.toUtf8());
}

QString Query::titleFromQueryUrl(const QUrl& url)
{
    QUrlQuery urlQuery(url);
    return urlQuery.queryItemValue(QStringLiteral("title"), QUrl::FullyDecoded);
}

bool Query::operator==(const Query& rhs) const
{
    if (rhs.d->m_limit != d->m_limit || rhs.d->m_offset != d->m_offset ||
        rhs.d->m_dayFilter != d->m_dayFilter || rhs.d->m_monthFilter != d->m_monthFilter ||
        rhs.d->m_yearFilter != d->m_yearFilter || rhs.d->m_includeFolder != d->m_includeFolder ||
        rhs.d->m_searchString != d->m_searchString ||
        rhs.d->m_sortingOption != d->m_sortingOption)
    {
        return false;
    }

    if (rhs.d->m_types.size() != d->m_types.size())
        return false;

    Q_FOREACH (const QString& type, rhs.d->m_types) {
        if (!d->m_types.contains(type))
            return false;
    }

    return d->m_term == rhs.d->m_term;
}

bool Query::operator!=(const Query& rhs) const
{
    return !(*this == rhs);
}

Query& Query::operator=(const Query& rhs)
{
    *d = *rhs.d;
    return *this;
}
