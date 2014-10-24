/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014  Vishesh Handa <me@vhanda.in>
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

#include "xapiansearchstore.h"
#include "xapianqueryparser.h"
#include "term.h"
#include "query.h"

#include <QVector>
#include <QStringList>
#include <QTime>
#include <QDebug>

#include <algorithm>

using namespace Baloo;

XapianSearchStore::XapianSearchStore(QObject* parent)
    : SearchStore(parent)
    , m_mutex(QMutex::Recursive)
    , m_nextId(1)
    , m_db(0)
{
}

XapianSearchStore::~XapianSearchStore()
{
    delete m_db;
}

void XapianSearchStore::setDbPath(const QString& path)
{
    m_dbPath = path;

    delete m_db;
    m_db = 0;

    try {
        m_db = new Xapian::Database(m_dbPath.toUtf8().constData());
    }
    catch (const Xapian::DatabaseOpeningError&) {
        qWarning() << "Xapian Database does not exist at " << m_dbPath;
    }
    catch (const Xapian::DatabaseCorruptError&) {
        qWarning() << "Xapian Database corrupted at " << m_dbPath;
    }
    catch (...) {
        qWarning() << "Random exception, but we do not want to crash";
    }
}

QString XapianSearchStore::dbPath()
{
    return m_dbPath;
}

Xapian::Query XapianSearchStore::toXapianQuery(Xapian::Query::op op, const QList<Term>& terms)
{
    Q_ASSERT_X(op == Xapian::Query::OP_AND || op == Xapian::Query::OP_OR,
               "XapianSearchStore::toXapianQuery", "The op must be AND / OR");

    QVector<Xapian::Query> queries;
    queries.reserve(terms.size());

    Q_FOREACH (const Term& term, terms) {
        Xapian::Query q = toXapianQuery(term);
        queries << q;
    }

    return Xapian::Query(op, queries.begin(), queries.end());
}

static Xapian::Query negate(bool shouldNegate, const Xapian::Query &query)
{
    if (shouldNegate) {
        return Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query::MatchAll, query);
    }
    return query;
}

Xapian::Query XapianSearchStore::toXapianQuery(const Term& term)
{
    if (term.operation() == Term::And) {
        return negate(term.isNegated(), toXapianQuery(Xapian::Query::OP_AND, term.subTerms()));
    }
    if (term.operation() == Term::Or) {
        return negate(term.isNegated(), toXapianQuery(Xapian::Query::OP_OR, term.subTerms()));
    }

    return negate(term.isNegated(), constructQuery(term.property(), term.value(), term.comparator()));
}

Xapian::Query XapianSearchStore::andQuery(const Xapian::Query& a, const Xapian::Query& b)
{
    if (a.empty() && !b.empty())
        return b;
    if (!a.empty() && b.empty())
        return a;
    if (a.empty() && b.empty())
        return Xapian::Query();
    else
        return Xapian::Query(Xapian::Query::OP_AND, a, b);
}

int XapianSearchStore::exec(const Query& query)
{
    if (!m_db)
        return 0;

    while (1) {
        try {
            QMutexLocker lock(&m_mutex);
            try {
                m_db->reopen();
            } catch (Xapian::DatabaseError& e) {
                qDebug() << "Failed to reopen database" << dbPath() << ":" <<  QString::fromStdString(e.get_msg());
                return 0;
            }

            Xapian::Query xapQ = toXapianQuery(query.term());
            // The term was not properly converted. Lets abort. The properties
            // must not exist
            if (!query.term().empty() && xapQ.empty()) {
                qDebug() << query.term() << "could not be processed. Aborting";
                return 0;
            }

            xapQ = andQuery(xapQ, convertTypes(query.types()));
            xapQ = andQuery(xapQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
            xapQ = applyIncludeFolder(xapQ, query.includeFolder());
            xapQ = finalizeQuery(xapQ);

            if (xapQ.empty()) {
                // Return all the results
                xapQ = Xapian::Query(std::string());
            }
            Xapian::Enquire enquire(*m_db);
            enquire.set_query(xapQ);

            if (query.sortingOption() == Query::SortNone) {
                enquire.set_weighting_scheme(Xapian::BoolWeight());
            }

            Result& res = m_queryMap[m_nextId++];
            res.mset = enquire.get_mset(query.offset(), query.limit());
            res.it = res.mset.begin();

            return m_nextId-1;
        }
        catch (const Xapian::DatabaseModifiedError&) {
            continue;
        }
        catch (const Xapian::Error&) {
            return 0;
        }
    }

    return 0;
}

void XapianSearchStore::close(int queryId)
{
    QMutexLocker lock(&m_mutex);
    m_queryMap.remove(queryId);
}

QByteArray XapianSearchStore::id(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Q_ASSERT_X(m_queryMap.contains(queryId), "FileSearchStore::id",
               "Passed a queryId which does not exist");

    const Result res = m_queryMap.value(queryId);
    if (!res.lastId)
        return QByteArray();

    return serialize(idPrefix(), res.lastId);
}

QString XapianSearchStore::filePath(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Result& res = m_queryMap[queryId];

    if (!res.lastId)
        return QString();

    if (!res.lastUrl.isEmpty())
        return res.lastUrl;

    res.lastUrl = constructFilePath(res.lastId);
    return res.lastUrl;
}

bool XapianSearchStore::next(int queryId)
{
    if (!m_db)
        return false;

    QMutexLocker lock(&m_mutex);
    Result& res = m_queryMap[queryId];

    bool atEnd = (res.it == res.mset.end());
    if (atEnd) {
        res.lastId = 0;
        res.lastUrl.clear();
    }
    else {
        res.lastId = *res.it;
        res.lastUrl.clear();
        ++res.it;
    }

    return !atEnd;
}

Xapian::Document XapianSearchStore::docForQuery(int queryId)
{
    if (!m_db)
        return Xapian::Document();

    QMutexLocker lock(&m_mutex);

    try {
        const Result& res = m_queryMap.value(queryId);
        if (!res.lastId)
            return Xapian::Document();

        return m_db->get_document(res.lastId);
    }
    catch (const Xapian::DocNotFoundError&) {
        return Xapian::Document();
    }
    catch (const Xapian::DatabaseModifiedError&) {
        m_db->reopen();
        return docForQuery(queryId);
    }
    catch (const Xapian::Error&) {
        return Xapian::Document();
    }
}

Xapian::Database* XapianSearchStore::xapianDb()
{
    return m_db;
}

Xapian::Query XapianSearchStore::constructFilterQuery(int year, int month, int day)
{
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(day);
    return Xapian::Query();
}

Xapian::Query XapianSearchStore::finalizeQuery(const Xapian::Query& query)
{
    return query;
}

Xapian::Query XapianSearchStore::applyIncludeFolder(const Xapian::Query& q, const QString& includeFolder)
{
    Q_UNUSED(includeFolder);
    return q;
}
