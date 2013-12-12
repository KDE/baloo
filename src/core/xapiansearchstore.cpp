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

#include "xapiansearchstore.h"
#include "term.h"
#include "query.h"

#include <QVector>
#include <QStringList>
#include <KDebug>

using namespace Baloo;

XapianSearchStore::XapianSearchStore(QObject* parent)
    : SearchStore(parent)
    , m_nextId(1)
    , m_mutex(QMutex::Recursive)
    , m_db(0)
{
}

void XapianSearchStore::setDbPath(const QString& path)
{
    m_dbPath = path;

    delete m_db;
    try {
        m_db = new Xapian::Database(m_dbPath.toStdString());
    }
    catch (const Xapian::DatabaseOpeningError&) {
        kError() << "Xapian Database does not exist at " << m_dbPath;
        m_db = 0;
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

Xapian::Query XapianSearchStore::toXapianQuery(const Term& term)
{
    if (term.operation() == Term::And) {
        return toXapianQuery(Xapian::Query::OP_AND, term.subTerms());
    }
    if (term.operation() == Term::Or) {
        return toXapianQuery(Xapian::Query::OP_OR, term.subTerms());
    }

    if (term.property().isEmpty())
        return Xapian::Query();

    return constructQuery(term.property(), term.value(), term.comparator());
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

    QMutexLocker lock(&m_mutex);
    m_db->reopen();

    Xapian::Query xapQ = toXapianQuery(query.term());
    if (query.searchString().size()) {
        std::string str = query.searchString().toStdString();

        Xapian::QueryParser parser;
        parser.set_database(*m_db);

        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        Xapian::Query q = parser.parse_query(str, flags);

        xapQ = andQuery(xapQ, q);
    }
    xapQ = andQuery(xapQ, convertTypes(query.types()));
    xapQ = andQuery(xapQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
    xapQ = finalizeQuery(xapQ);

    Xapian::Enquire enquire(*m_db);
    kDebug() << xapQ.get_description().c_str();
    enquire.set_query(xapQ);

    Result& res = m_queryMap[m_nextId++];
    res.mset = enquire.get_mset(0, query.limit());
    res.it = res.mset.begin();

    return m_nextId-1;
}

void XapianSearchStore::close(int queryId)
{
    QMutexLocker lock(&m_mutex);
    m_queryMap.remove(queryId);
}

Item::Id XapianSearchStore::id(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Q_ASSERT_X(m_queryMap.contains(queryId), "FileSearchStore::id",
               "Passed a queryId which does not exist");

    const Result res = m_queryMap.value(queryId);
    if (!res.lastId)
        return QByteArray();

    return serialize(idPrefix(), res.lastId);
}

QUrl XapianSearchStore::url(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Result& res = m_queryMap[queryId];

    if (!res.lastId)
        return QUrl();

    if (!res.lastUrl.isEmpty())
        return res.lastUrl;

    res.lastUrl = constructUrl(res.lastId);
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
        res.it++;
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
