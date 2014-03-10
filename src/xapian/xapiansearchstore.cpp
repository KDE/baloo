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

#include "xapiansearchstore.h"
#include "term.h"
#include "query.h"

#include <QVector>
#include <QStringList>
#include <QTime>
#include <KDebug>

#include <algorithm>

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
        m_db = new Xapian::Database(m_dbPath.toUtf8().constData());
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

    if (term.property().isEmpty())
        return Xapian::Query();

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

namespace {
    struct Term {
        std::string t;
        uint count;

        // pop_heap pops the largest element, we want the smallest to be popped
        bool operator < (const Term& rhs) const {
            return count > rhs.count;
        }
    };

    Xapian::Query makeQuery(const QString& string, Xapian::Database* db)
    {
        // Lets just keep the top x (+1 for push_heap)
        static const int MaxTerms = 100;
        QList<Term> topTerms;
        topTerms.reserve(MaxTerms + 1);

        const std::string stdString(string.toLower().toUtf8().constData());
        Xapian::TermIterator it = db->allterms_begin(stdString);
        Xapian::TermIterator end = db->allterms_end(stdString);
        for (; it != end; it++) {
            Term term;
            term.t = *it;
            term.count = db->get_collection_freq(term.t);

            if (topTerms.size() < MaxTerms) {
                topTerms.push_back(term);
                std::push_heap(topTerms.begin(), topTerms.end());
            }
            else {
                // Remove the term with the min count
                topTerms.push_back(term);
                std::push_heap(topTerms.begin(), topTerms.end());

                std::pop_heap(topTerms.begin(), topTerms.end());
                topTerms.pop_back();
            }
        }

        QVector<std::string> termStrings;
        termStrings.reserve(topTerms.size());

        Q_FOREACH (const Term& term, topTerms) {
            termStrings << term.t;
        }

        Xapian::Query finalQ(Xapian::Query::OP_SYNONYM, termStrings.begin(), termStrings.end());
        return finalQ;
    }
}


Xapian::Query XapianSearchStore::constructSearchQuery(const QString& str)
{
    QVector<Xapian::Query> queries;
    QRegExp splitRegex("[\\s.]");
    QStringList list = str.split(splitRegex, QString::SkipEmptyParts);

    QMutableListIterator<QString> iter(list);
    while (iter.hasNext()) {
        const QString str = iter.next();
        if (str.size() <= 3) {
            queries << makeQuery(str, m_db);
            iter.remove();
        }
    }

    if (!list.isEmpty()) {
        std::string stdStr(list.join(" ").toUtf8().constData());

        Xapian::QueryParser parser;
        parser.set_database(*m_db);
        parser.set_default_op(Xapian::Query::OP_AND);

        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        queries << parser.parse_query(stdStr, flags);
    }

    return Xapian::Query(Xapian::Query::OP_AND, queries.begin(), queries.end());
}

int XapianSearchStore::exec(const Query& query)
{
    if (!m_db)
        return 0;

    QMutexLocker lock(&m_mutex);
    m_db->reopen();

    QTime queryGenerationTimer;
    queryGenerationTimer.start();

    Xapian::Query xapQ = toXapianQuery(query.term());
    if (query.searchString().size()) {
        QString str = query.searchString();

        Xapian::Query q = constructSearchQuery(str);
        xapQ = andQuery(xapQ, q);
    }
    xapQ = andQuery(xapQ, convertTypes(query.types()));
    xapQ = andQuery(xapQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
    xapQ = applyCustomOptions(xapQ, query.customOptions());
    xapQ = finalizeQuery(xapQ);

    Xapian::Enquire enquire(*m_db);
    kDebug() << xapQ.get_description().c_str();
    enquire.set_query(xapQ);

    kDebug() << "Query Generation" << queryGenerationTimer.elapsed();

    Result& res = m_queryMap[m_nextId++];
    QTime timer;
    timer.start();
    res.mset = enquire.get_mset(query.offset(), query.limit());
    res.it = res.mset.begin();

    kDebug() << "Exec" << timer.elapsed() << "msecs";
    return m_nextId-1;
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

Xapian::Query XapianSearchStore::applyCustomOptions(const Xapian::Query& q, const QVariantHash& options)
{
    Q_UNUSED(options);
    return q;
}
