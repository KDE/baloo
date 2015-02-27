/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
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

#include "lucenesearchstore.h"
#include "query.h"


using namespace Baloo;

LuceneSearchStore::LuceneSearchStore(QObject* parent)
    : SearchStore(parent)
    , m_mutex(QMutex::Recursive)
    , m_nextId(1)
{
}

LuceneSearchStore::~LuceneSearchStore()
{
}


void LuceneSearchStore::setIndexPath(const QString& path)
{
    m_indexPath = path;

    m_reader = Lucene::IndexReader::open( Lucene::FSDirectory::open(path.toStdWString()) );
}

QString LuceneSearchStore::dbPath()
{
    return m_indexPath;
}

Lucene::QueryPtr LuceneSearchStore::toLuceneQuery(Lucene::BooleanClause::Occur op, const QList<Term>& terms)
{
    Lucene::BooleanQueryPtr boolQuery = Lucene::newLucene<Lucene::BooleanQuery>();
    Q_FOREACH (const Term& term, terms) {
        Lucene::QueryPtr q = toLuceneQuery(term);
        boolQuery->add(q, op);
    }
    Lucene::QueryPtr res = boolQuery->rewrite(m_reader);
    return  res;
}

static Lucene::QueryPtr negate(bool shouldNegate, Lucene::QueryPtr query, const Lucene::IndexReaderPtr& m_reader)
{
    if (shouldNegate) {
        Lucene::BooleanQueryPtr boolQuery = Lucene::newLucene<Lucene::BooleanQuery>();
        boolQuery->add(query, Lucene::BooleanClause::MUST_NOT);
        query = boolQuery->rewrite(m_reader);
    }
    return query;
}

Lucene::QueryPtr LuceneSearchStore::toLuceneQuery(const Term& term)
{
    Lucene::QueryPtr res;
    if (term.operation() == Term::And) {
        res = negate(term.isNegated(), toLuceneQuery(Lucene::BooleanClause::MUST, term.subTerms()), m_reader);
    }
    else if (term.operation() == Term::Or) {
        res = negate(term.isNegated(), toLuceneQuery(Lucene::BooleanClause::SHOULD, term.subTerms()), m_reader);
    }
    else {
        res = constructQuery(term.property(), term.value(), term.comparator());
    }
    return res;
}

Lucene::QueryPtr LuceneSearchStore::andQuery(const Lucene::QueryPtr& a, const Lucene::QueryPtr& b)
{
    Lucene::QueryPtr res;
    if (isEmpty(a) && !isEmpty(b)) {
        res = b;
    }
    else if (!isEmpty(a) && isEmpty(b)) {
        res = a;
    }
    else if (!isEmpty(a) && !isEmpty(b)) {
        Lucene::BooleanQueryPtr boolQuery = Lucene::newLucene<Lucene::BooleanQuery>();
        boolQuery->add(a, Lucene::BooleanClause::MUST);
        boolQuery->add(b, Lucene::BooleanClause::MUST);
        res = boolQuery->rewrite(m_reader);
    }
    return res;
}

int LuceneSearchStore::exec(const Query& query)
{
    if (!m_reader)  {
        return 0;
    }
    QMutexLocker lock(&m_mutex);
    m_reader = m_reader->reopen();

    Lucene::QueryPtr luQ = toLuceneQuery(query.term());
    luQ = andQuery(luQ, convertTypes(query.types()));
    luQ = andQuery(luQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
    luQ = applyIncludeFolder(luQ, query.includeFolder());
    luQ = finalizeQuery(luQ);

    if (isEmpty(luQ)) {
        luQ = Lucene::newLucene<Lucene::MatchAllDocsQuery>();
    }

    Lucene::SearcherPtr searcher = Lucene::newLucene<Lucene::IndexSearcher>(m_reader);
    Result& res = m_queryMap[m_nextId++];
    int numResults = 100;
    Lucene::TopDocsPtr topDocs = searcher->search(luQ, numResults);
    res.hits = topDocs->scoreDocs;
    res.it = res.hits.begin();
    return m_nextId - 1;
}

bool LuceneSearchStore::isEmpty(const Lucene::QueryPtr& query)
{
    return query->toString().empty();
}

void LuceneSearchStore::close(int queryId)
{
    QMutexLocker lock(&m_mutex);
    m_queryMap.remove(queryId);
}

QByteArray LuceneSearchStore::id(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Q_ASSERT_X(m_queryMap.contains(queryId), "FileSearchStore::id",
               "Passed a queryId which does not exist");

    const Result res = m_queryMap.value(queryId);
    if (!res.lastId)
        return QByteArray();

    return serialize(idPrefix(), res.lastId);
}

QString LuceneSearchStore::filePath(int queryId)
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

bool LuceneSearchStore::next(int queryId)
{
    if (!m_reader)
        return false;

    QMutexLocker lock(&m_mutex);
    Result& res = m_queryMap[queryId];

    bool atEnd = (res.it == res.hits.end());
    if (atEnd) {
        res.lastId = 0;
        res.lastUrl.clear();
    }
    else {
        res.lastId = (*res.it)->doc;
        res.lastUrl.clear();
        ++res.it;
    }

    return !atEnd;
}

Lucene::DocumentPtr LuceneSearchStore::docForQuery(int queryId)
{
    if (!m_reader) {
        Lucene::DocumentPtr doc = Lucene::newLucene<Lucene::Document>();
        return doc;
    }

    QMutexLocker lock(&m_mutex);

    const Result& res = m_queryMap.value(queryId);
    if (!res.lastId) {
        Lucene::DocumentPtr doc = Lucene::newLucene<Lucene::Document>();
        return doc;
    }

    return m_reader->document(res.lastId);
}

Lucene::IndexReaderPtr LuceneSearchStore::luceneIndex()
{
    return m_reader;
}

Lucene::QueryPtr LuceneSearchStore::constructFilterQuery(int year, int month, int day)
{
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(day);
    Lucene::QueryPtr query = Lucene::newLucene<Lucene::BooleanQuery>();
    return query;
}

Lucene::QueryPtr LuceneSearchStore::finalizeQuery(const Lucene::QueryPtr& query)
{
    return query;
}

Lucene::QueryPtr LuceneSearchStore::applyIncludeFolder(const Lucene::QueryPtr& q, QString includeFolder)
{
    Q_UNUSED(includeFolder);
    return q;
}
