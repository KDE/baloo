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

#include "filesearchstore.h"
#include "item.h"
#include "term.h"
#include "query.h"

#include <xapian.h>
#include <QVector>

#include <KStandardDirs>
#include <KDebug>

using namespace Baloo;

FileSearchStore::FileSearchStore(QObject* parent, const QVariantList&)
    : SearchStore(parent)
{
    m_dbPath = KStandardDirs::locateLocal("data", "baloo/file/");
    m_nextId = 1;
}

QStringList FileSearchStore::types()
{
    return QStringList() << "File";
}

Xapian::Query FileSearchStore::toXapianQuery(Xapian::Query::op op, const QList<Term>& terms)
{
    Q_ASSERT_X(op == Xapian::Query::OP_AND || op == Xapian::Query::OP_OR,
               "FileSearchStore::toXapianQuery", "The op must be AND / OR");

    QVector<Xapian::Query> queries;
    queries.reserve(terms.size());

    Q_FOREACH (const Term& term, terms) {
        Xapian::Query q = toXapianQuery(term);
        queries << q;
    }

    return Xapian::Query(op, queries.begin(), queries.end());
}

Xapian::Query FileSearchStore::toXapianQuery(const Term& term)
{
    if (term.operation() == Term::And) {
        return toXapianQuery(Xapian::Query::OP_AND, term.subTerms());
    }
    if (term.operation() == Term::Or) {
        return toXapianQuery(Xapian::Query::OP_OR, term.subTerms());
    }

    if (term.property().isEmpty())
        return Xapian::Query();

    // FIXME: Need some way to check if only a property exists!
    if (!term.value().isNull()) {
        return Xapian::Query();
    }

    // Both property and value are non empty
    // FIXME: How to convert the property to the appropriate prefix?
    if (term.comparator() == Term::Contains) {
        Xapian::QueryParser parser;

        std::string prefix = term.property().toUpper().toStdString();
        std::string str = term.value().toString().toStdString();
        return parser.parse_query(str, Xapian::QueryParser::FLAG_DEFAULT, prefix);
    }

    // FIXME: We use equals in all other conditions
    //if (term.comparator() == Term::Equal) {
        return Xapian::Query(term.value().toString().toStdString());
    //}
}


int FileSearchStore::exec(const Query& query)
{
    Xapian::Query xapQ = toXapianQuery(query.term());
    if (query.searchString().size()) {
        std::string str = query.searchString().toStdString();

        Xapian::QueryParser parser;
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        Xapian::Query q = parser.parse_query(str, flags);

        if (xapQ.empty())
            xapQ = q;
        else
            xapQ = Xapian::Query(Xapian::Query::OP_AND, q, xapQ);
    }

    Xapian::Database db(m_dbPath.toStdString());
    Xapian::Enquire enquire(db);
    enquire.set_query(xapQ);

    Iter& it = m_queryMap[m_nextId++];
    it.mset = enquire.get_mset(0, query.limit());
    it.it = it.mset.begin();

    return m_nextId-1;
}

void FileSearchStore::close(int queryId)
{
    m_queryMap.remove(queryId);
}

Item::Id FileSearchStore::id(int queryId)
{
    Iter& it = m_queryMap[queryId];
    uint id = *(it.it);

    return serialize("file", id);
}

bool FileSearchStore::next(int queryId)
{
    Iter& it = m_queryMap[queryId];
    it.it++;
    return it.it != it.mset.end();
}

BALOO_EXPORT_SEARCHSTORE(Baloo::FileSearchStore, "baloo_filesearchstore")
