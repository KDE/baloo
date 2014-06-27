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
#include "searchstore.h"

#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QList>
#include <QUrlQuery>

#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>

using namespace Baloo;

const int defaultLimit = 100000;

class Baloo::Query::Private {
public:
    Private() {
        m_limit = defaultLimit;
        m_offset = 0;
        m_yearFilter = -1;
        m_monthFilter = -1;
        m_dayFilter = -1;
    }
    Term m_term;

    QStringList m_types;
    QString m_searchString;
    uint m_limit;
    uint m_offset;

    int m_yearFilter;
    int m_monthFilter;
    int m_dayFilter;

    QVariantMap m_customOptions;
};

Query::Query()
    : d(new Private)
{
}

Query::Query(const Term& t)
    : d(new Private)
{
    d->m_term = t;
}

Query::Query(const Query& rhs)
    : d(new Private(*rhs.d))
{
}

Query::~Query()
{
    delete d;
}

void Query::setTerm(const Term& t)
{
    d->m_term = t;
}

Term Query::term() const
{
    return d->m_term;
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

void Query::addCustomOption(const QString& option, const QVariant& value)
{
    d->m_customOptions.insert(option, value);
}

QVariant Query::customOption(const QString& option) const
{
    return d->m_customOptions.value(option);
}

QVariantMap Query::customOptions() const
{
    return d->m_customOptions;
}

void Query::removeCustomOption(const QString& option)
{
    d->m_customOptions.remove(option);
}

Q_GLOBAL_STATIC_WITH_ARGS(SearchStore::List, s_searchStores, (SearchStore::searchStores()))

ResultIterator Query::exec()
{
    // vHanda: Maybe this should default to allow searches on all search stores?
    Q_ASSERT_X(!types().isEmpty(), "Baloo::Query::exec", "A query is being initialized without a type");
    if (types().isEmpty())
        return ResultIterator();

    SearchStore* storeMatch = 0;
    Q_FOREACH (QSharedPointer<SearchStore> store, *s_searchStores) {
        bool matches = true;
        Q_FOREACH (const QString& type, types()) {
            if (!store->types().contains(type)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            storeMatch = store.data();
            break;
        }
    }

    if (!storeMatch)
        return ResultIterator();

    int id = storeMatch->exec(*this);
    return ResultIterator(id, storeMatch);
}

QByteArray Query::toJSON()
{
    QVariantMap map;

    if (!d->m_types.isEmpty())
        map[QLatin1String("type")] = d->m_types;

    if (d->m_limit != defaultLimit)
        map[QLatin1String("limit")] = d->m_limit;

    if (d->m_offset)
        map[QLatin1String("offset")] = d->m_offset;

    if (!d->m_searchString.isEmpty())
        map[QLatin1String("searchString")] = d->m_searchString;

    if (d->m_term.isValid())
        map[QLatin1String("term")] = QVariant(d->m_term.toVariantMap());

    if (d->m_yearFilter >= 0)
        map[QLatin1String("yearFilter")] = d->m_yearFilter;
    if (d->m_monthFilter >= 0)
        map[QLatin1String("monthFilter")] = d->m_monthFilter;
    if (d->m_dayFilter >= 0)
        map[QLatin1String("dayFilter")] = d->m_dayFilter;

    if (d->m_customOptions.size())
        map[QLatin1String("customOptions")] = d->m_customOptions;

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
    query.d->m_types = map[QLatin1String("type")].toStringList();

    if (map.contains(QLatin1String("limit")))
        query.d->m_limit = map[QLatin1String("limit")].toUInt();
    else
        query.d->m_limit = defaultLimit;

    query.d->m_offset = map[QLatin1String("offset")].toUInt();
    query.d->m_searchString = map[QLatin1String("searchString")].toString();
    query.d->m_term = Term::fromVariantMap(map[QLatin1String("term")].toMap());

    if (map.contains(QLatin1String("yearFilter")))
        query.d->m_yearFilter = map[QLatin1String("yearFilter")].toInt();
    if (map.contains(QLatin1String("monthFilter")))
        query.d->m_monthFilter = map[QLatin1String("monthFilter")].toInt();
    if (map.contains(QLatin1String("dayFilter")))
        query.d->m_dayFilter = map[QLatin1String("dayFilter")].toInt();

    if (map.contains(QLatin1String("customOptions"))) {
        QVariant var = map[QLatin1String("customOptions")];
        if (var.type() == QVariant::Map) {
            query.d->m_customOptions = map[QLatin1String("customOptions")].toMap();
        }
        else if (var.type() == QVariant::Hash) {
            QVariantHash hash = var.toHash();

            QHash<QString, QVariant>::const_iterator it = hash.constBegin();
            for (; it != hash.constEnd(); ++it)
                query.d->m_customOptions.insert(it.key(), it.value());
        }
    }

    return query;
}

QUrl Query::toSearchUrl(const QString& title)
{
    QUrl url;
    url.setScheme(QLatin1String("baloosearch"));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("json"), QString::fromUtf8(toJSON()));

    if (title.size())
        urlQuery.addQueryItem(QLatin1String("title"), title);

    url.setQuery(urlQuery);
    return url;
}

Query Query::fromSearchUrl(const QUrl& url)
{
    if (url.scheme() != QLatin1String("baloosearch"))
        return Query();

    QUrlQuery urlQuery(url);
    QString jsonString = urlQuery.queryItemValue(QLatin1String("json"));
    return Query::fromJSON(jsonString.toUtf8());
}

QString Query::titleFromQueryUrl(const QUrl& url)
{
    QUrlQuery urlQuery(url);
    return urlQuery.queryItemValue(QLatin1String("title"));
}

bool Query::operator==(const Query& rhs) const
{
    if (rhs.d->m_limit != d->m_limit || rhs.d->m_offset != d->m_offset ||
        rhs.d->m_dayFilter != d->m_dayFilter || rhs.d->m_monthFilter != d->m_monthFilter ||
        rhs.d->m_yearFilter != d->m_yearFilter || rhs.d->m_customOptions != d->m_customOptions ||
        rhs.d->m_searchString != d->m_searchString )
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

Query& Query::operator=(const Query& rhs)
{
    *d = *rhs.d;
    return *this;
}
