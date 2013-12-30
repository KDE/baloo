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
#include <QList>

#include <KDebug>

#include <qjson/serializer.h>
#include <qjson/parser.h>

using namespace Baloo;

class Baloo::Query::Private {
public:
    Private() {
        m_limit = 100000;
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

    QVariantHash m_customOptions;
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
    d->m_types << type.split('/', QString::SkipEmptyParts);
}

void Query::addTypes(const QStringList& typeList)
{
    Q_FOREACH (const QString& type, typeList) {
        addType(type);
    }
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

void Query::addRelation(const Relation& rel)
{
    //TODO:
}

void Query::setRelations(const QList<Relation>& rel)
{
    //TODO:
}

QList<Relation> Query::relations() const
{
    //TODO:
    return QList<Relation>();
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

QVariantHash Query::customOptions() const
{
    return d->m_customOptions;
}

void Query::removeCustomOption(const QString& option)
{
    d->m_customOptions.remove(option);
}

ResultIterator Query::exec()
{
    // vHanda: Maybe this should default to allow searches on all search stores?
    Q_ASSERT_X(!types().isEmpty(), "Baloo::Query::exec", "A query is being initialized without a type");
    if (types().isEmpty())
        return ResultIterator();

    static QList<SearchStore*> stores = SearchStore::searchStores();

    SearchStore* storeMatch = 0;
    Q_FOREACH (SearchStore* store, stores) {
        Q_FOREACH (const QString& type, types()) {
            if (store->types().contains(type)) {
                storeMatch = store;
                break;
            }
        }

        if (storeMatch)
            break;
    }

    if (!storeMatch)
        return ResultIterator();

    int id = storeMatch->exec(*this);
    return ResultIterator(id, storeMatch);
}

QByteArray Query::toJSON()
{
    QVariantMap map;
    map["type"] = d->m_types;
    map["limit"] = d->m_limit;
    map["offset"] = d->m_offset;
    map["searchString"] = d->m_searchString;
    map["term"] = QVariant(d->m_term.toVariantMap());

    if (d->m_yearFilter >= 0)
        map["yearFilter"] = d->m_yearFilter;
    if (d->m_monthFilter >= 0)
        map["monthFilter"] = d->m_monthFilter;
    if (d->m_dayFilter >= 0)
        map["dayFilter"] = d->m_dayFilter;

    if (d->m_customOptions.size())
        map["customOptions"] = d->m_customOptions;

    QJson::Serializer serializer;
    return serializer.serialize(map);
}

// static
Query Query::fromJSON(const QByteArray& arr)
{
    QJson::Parser parser;
    QVariantMap map = parser.parse(arr).toMap();

    Query query;
    query.d->m_types = map["type"].toStringList();
    query.d->m_limit = map["limit"].toUInt();
    query.d->m_offset = map["offset"].toUInt();
    query.d->m_searchString = map["searchString"].toString();
    query.d->m_term = Term::fromVariantMap(map["term"].toMap());

    if (map.contains("yearFilter"))
        query.d->m_yearFilter = map["yearFilter"].toInt();
    if (map.contains("monthFilter"))
        query.d->m_monthFilter = map["monthFilter"].toInt();
    if (map.contains("dayFilter"))
        query.d->m_dayFilter = map["dayFilter"].toInt();

    if (map.contains("customOptions"))
        query.d->m_customOptions = map["customOptions"].toHash();

    return query;
}

QUrl Query::toSearchUrl(const QString& title)
{
    QUrl url;
    url.setScheme(QLatin1String("baloosearch"));
    url.addQueryItem(QLatin1String("json"), QString::fromUtf8(toJSON()));

    if (title.size())
        url.addQueryItem(QLatin1String("title"), title);

    return url;
}

Query Query::fromSearchUrl(const QUrl& url)
{
    if (url.scheme() != QLatin1String("baloosearch"))
        return Query();

    QString jsonString = url.queryItemValue(QLatin1String("json"));
    return Query::fromJSON(jsonString.toUtf8());
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
