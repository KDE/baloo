/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015  Vishesh Handa <vhanda@kde.org>
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

#include "searchstore.h"
#include "term.h"

#include "database.h"
#include "transaction.h"
#include "enginequery.h"
#include "queryparser.h"
#include "termgenerator.h"

#include <QStandardPaths>

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>
#include <KFileMetaData/Types>

using namespace Baloo;

SearchStore::SearchStore()
    : m_db(0)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
    m_db = new Database(path);
    m_db->open();

    m_prefixes.insert(QByteArray("filename"), QByteArray("F"));
    m_prefixes.insert(QByteArray("mimetype"), QByteArray("M"));
    m_prefixes.insert(QByteArray("rating"), QByteArray("R"));
    m_prefixes.insert(QByteArray("tag"), QByteArray("TA"));
    m_prefixes.insert(QByteArray("tags"), QByteArray("TA"));
    m_prefixes.insert(QByteArray("usercomment"), QByteArray("C"));
    m_prefixes.insert(QByteArray("type"), QByteArray("T"));
    m_prefixes.insert(QByteArray("kind"), QByteArray("T"));
}

SearchStore::~SearchStore()
{
    delete m_db;
}

QVector<quint64> SearchStore::exec(const Term& term, int limit)
{
    EngineQuery query = constructQuery(term);
    if (query.empty()) {
        return QVector<quint64>();
    }

    Transaction tr(m_db, Transaction::ReadOnly);
    return tr.exec(query, limit);
}

QString SearchStore::filePath(quint64 id)
{
    Q_ASSERT(id > 0);

    Transaction tr(m_db, Transaction::ReadOnly);
    return tr.documentUrl(id);
}

QByteArray SearchStore::fetchPrefix(const QByteArray& property) const
{
    auto it = m_prefixes.constFind(property.toLower());
    if (it != m_prefixes.constEnd()) {
        return it.value();
    }
    else {
        KFileMetaData::PropertyInfo pi = KFileMetaData::PropertyInfo::fromName(property);
        if (pi.property() == KFileMetaData::Property::Empty) {
            qDebug() << "Property" << property << "not found";
            return QByteArray();
        }
        int propPrefix = static_cast<int>(pi.property());
        return 'X' + QByteArray::number(propPrefix);
    }

}

EngineQuery SearchStore::constructQuery(const Term& term)
{
    if (term.operation() == Term::And || term.operation() == Term::Or) {
        QVector<EngineQuery> vec;
        for (const Term& t : term.subTerms()) {
            EngineQuery q = constructQuery(t);
            if (!q.empty())
                vec << q;
        }

        if (vec.isEmpty()) {
            return EngineQuery();
        }

        return EngineQuery(vec, term.operation() == Term::And ? EngineQuery::And : EngineQuery::Or);
    }

    const QVariant value = term.value();
    if (value.isNull()) {
        return EngineQuery();
    }

    QByteArray property = term.property().toLower().toUtf8();
    // TODO:
    // Handle Ratings - or generic integer queries
    // Handle FileNames
    // Handle "modified"

    if (property == "type" || property == "kind") {
        KFileMetaData::TypeInfo ti = KFileMetaData::TypeInfo::fromName(value.toString());
        if (ti == KFileMetaData::Type::Empty) {
            qDebug() << "Type" << value.toString() << "does not exist";
            return EngineQuery();
        }
        int num = static_cast<int>(ti.type());

        return EngineQuery('T' + QByteArray::number(num));
    }

    auto com = term.comparator();
    if (com == Term::Contains) {
        QByteArray prefix;
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                return EngineQuery();
            }
        }

        QueryParser parser;
        return parser.parseQuery(value.toString(), prefix);
    }

    if (com == Term::Equal) {
        // We use the TermGenerator to normalize the words in the value and to
        // split it into other words. If we split the words, we then add them as a
        // phrase query.
        QStringList terms = TermGenerator::termList(value.toString());

        QByteArray prefix;
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                return EngineQuery();
            }
        }

        QVector<EngineQuery> queries;
        int position = 1;
        for (const QString& term : terms) {
            QByteArray arr = (prefix + term).toUtf8();
            queries << EngineQuery(arr, position++);
        }

        if (queries.isEmpty()) {
            return EngineQuery();
        } else if (queries.size() == 1) {
            return queries.first();
        } else {
            return EngineQuery(queries, EngineQuery::Phrase);
        }
    }

    return EngineQuery();
}

