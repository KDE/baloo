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
#include "global.h"

#include "database.h"
#include "transaction.h"
#include "enginequery.h"
#include "queryparser.h"
#include "termgenerator.h"
#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "idutils.h"

#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>
#include <KFileMetaData/Types>

#include <algorithm>

using namespace Baloo;

SearchStore::SearchStore()
    : m_db(nullptr)
{
    m_db = globalDatabaseInstance();
    if (!m_db->open(Database::ReadOnlyDatabase)) {
        m_db = nullptr;
    }

    m_prefixes.insert(QByteArray("filename"), QByteArray("F"));
    m_prefixes.insert(QByteArray("mimetype"), QByteArray("M"));
    m_prefixes.insert(QByteArray("rating"), QByteArray("R"));
    m_prefixes.insert(QByteArray("tag"), QByteArray("TAG-"));
    m_prefixes.insert(QByteArray("tags"), QByteArray("TA"));
    m_prefixes.insert(QByteArray("usercomment"), QByteArray("C"));
}

SearchStore::~SearchStore()
{
}

// Return the result with-in [offset, offset + limit)
QStringList SearchStore::exec(const Term& term, uint offset, int limit, bool sortResults)
{
    if (!m_db || !m_db->isOpen()) {
        return QStringList();
    }

    Transaction tr(m_db, Transaction::ReadOnly);
    QScopedPointer<PostingIterator> it(constructQuery(&tr, term));
    if (!it) {
        return QStringList();
    }

    if (sortResults) {
        QVector<quint64> resultIds;
        while (it->next()) {
            quint64 id = it->docId();
            resultIds << id;

            Q_ASSERT(id > 0);
        }

        // No enough result within range, no need to sort.
        if (offset >= static_cast<uint>(resultIds.size())) {
            return QStringList();
        }

        auto compFunc = [&tr](const quint64 lhs, const quint64 rhs) {
            return tr.documentTimeInfo(lhs).mTime > tr.documentTimeInfo(rhs).mTime;
        };

        std::sort(resultIds.begin(), resultIds.end(), compFunc);
        if (limit < 0) {
            limit = resultIds.size();
        }

        QStringList results;
        const uint end = qMin(static_cast<uint>(resultIds.size()), offset + static_cast<uint>(limit));
        for (uint i = offset; i < end; i++) {
            const quint64 id = resultIds[i];
            const QString filePath = tr.documentUrl(id);

            results << filePath;
        }

        return results;
    }
    else {
        uint i = 0;
        QStringList results;
        const uint end = offset + static_cast<uint>(limit);

        while (it->next() && (limit < 0 || i < end)) {
            quint64 id = it->docId();
            Q_ASSERT(id > 0);

            if (i >= offset) {
                results << tr.documentUrl(it->docId());
                Q_ASSERT(!results.last().isEmpty());
            }

            i++;
        }

        return results;
    }
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
        return 'X' + QByteArray::number(propPrefix) + '-';
    }

}

PostingIterator* SearchStore::constructQuery(Transaction* tr, const Term& term)
{
    Q_ASSERT(tr);

    if (term.operation() == Term::And || term.operation() == Term::Or) {
        QList<Term> subTerms = term.subTerms();
        QVector<PostingIterator*> vec;
        vec.reserve(subTerms.size());

        for (const Term& t : term.subTerms()) {
            vec << constructQuery(tr, t);
        }

        if (vec.isEmpty()) {
            return nullptr;
        }

        if (term.operation() == Term::And) {
            return new AndPostingIterator(vec);
        } else {
            return new OrPostingIterator(vec);
        }
    }

    if (term.value().isNull()) {
        return nullptr;
    }
    Q_ASSERT(term.value().isValid());
    Q_ASSERT(term.comparator() != Term::Auto);
    Q_ASSERT(term.comparator() == Term::Contains ? term.value().type() == QVariant::String : true);

    const QVariant value = term.value();
    const QByteArray property = term.property().toLower().toUtf8();

    if (property == "type" || property == "kind") {
        EngineQuery q = constructTypeQuery(value.toString());
        return tr->postingIterator(q);
    }
    else if (property == "includefolder") {
        const QByteArray folder = QFile::encodeName(QFileInfo(value.toString()).canonicalFilePath());

        Q_ASSERT(!folder.isEmpty());
        Q_ASSERT(folder.startsWith('/'));

        quint64 id = filePathToId(folder);
        if (!id) {
            qDebug() << "Folder" << value.toString() << "does not exist";
            return nullptr;
        }

        return tr->docUrlIter(id);
    }
    else if (property == "modified" || property == "mtime") {
        if (value.type() == QVariant::ByteArray) {
            QByteArray ba = value.toByteArray();
            Q_ASSERT(ba.size() >= 4);

            int year = ba.mid(0, 4).toInt();
            int month = ba.mid(4, 2).toInt();
            int day = ba.mid(6, 2).toInt();

            Q_ASSERT(year);

            // uses 0 to represent whole month or whole year
            month = month >= 0 && month <= 12 ? month : 0;
            day = day >= 0 && day <= 31 ? day : 0;

            QDate startDate(year, month ? month : 1, day ? day : 1);
            QDate endDate(startDate);

            if (month == 0) {
                endDate.setDate(endDate.year(), 12, 31);
            } else if (day == 0) {
                endDate.setDate(endDate.year(), endDate.month(), endDate.daysInMonth());
            }

            return tr->mTimeRangeIter(QDateTime(startDate).toTime_t(), QDateTime(endDate, QTime(23, 59, 59)).toTime_t());
        }
        else if (value.type() == QVariant::Date || value.type() == QVariant::DateTime) {
            const QDateTime dt = value.toDateTime();
            return constructMTimeQuery(tr, dt, term.comparator());
        }
        else {
            Q_ASSERT_X(0, "SearchStore::constructQuery", "modified property must contain date/datetime values");
        }
    }
    else if (property == "rating") {
        bool okay = false;
        int rating = value.toInt(&okay);
        if (!okay) {
            qDebug() << "Rating comparisons must be with an integer";
            return nullptr;
        }

        PostingDB::Comparator pcom;
        if (term.comparator() == Term::Greater || term.comparator() == Term::GreaterEqual) {
            pcom = PostingDB::GreaterEqual;
            if (term.comparator() == Term::Greater && rating)
                rating++;
        }
        else if (term.comparator() == Term::Less || term.comparator() == Term::LessEqual) {
            pcom = PostingDB::LessEqual;
            if (term.comparator() == Term::Less)
                rating--;
        }
        else if (term.comparator() == Term::Equal) {
            EngineQuery q = constructEqualsQuery("R", value.toString());
            return tr->postingIterator(q);
        }
        else {
            Q_ASSERT(0);
            return nullptr;
        }

        const QByteArray prefix = "R";
        const QByteArray val = QByteArray::number(rating);
        return tr->postingCompIterator(prefix, val, pcom);
    } else if (property == "tag") {
        if (term.comparator() == Term::Equal) {
            const QByteArray prefix = "TAG-";
            EngineQuery q = EngineQuery(prefix + value.toByteArray());
            return tr->postingIterator(q);
        } else if (term.comparator() == Term::Contains) {
            const QByteArray prefix = "TA";
            EngineQuery q = constructEqualsQuery(prefix, value.toString());
            return tr->postingIterator(q);
        } else {
            Q_ASSERT(0);
            return nullptr;
        }
    }

    QByteArray prefix;
    if (!property.isEmpty()) {
        prefix = fetchPrefix(property);
        if (prefix.isEmpty()) {
            return nullptr;
        }
    }

    auto com = term.comparator();
    if (com == Term::Contains) {
        EngineQuery q = constructContainsQuery(prefix, value.toString());
        return tr->postingIterator(q);
    }

    if (com == Term::Equal) {
        EngineQuery q = constructEqualsQuery(prefix, value.toString());
        return tr->postingIterator(q);
    }

    QVariant val = term.value();
    if (val.type() == QVariant::Int) {
        int intVal = value.toInt();

        PostingDB::Comparator pcom;
        if (term.comparator() == Term::Greater || term.comparator() == Term::GreaterEqual) {
            pcom = PostingDB::GreaterEqual;
            if (term.comparator() == Term::Greater && intVal)
                intVal++;
        }
        else if (term.comparator() == Term::Less || term.comparator() == Term::LessEqual) {
            pcom = PostingDB::LessEqual;
            if (term.comparator() == Term::Less)
                intVal--;
        }
        else {
            Q_ASSERT(0);
            return nullptr;
        }

        return tr->postingCompIterator(prefix, QByteArray::number(intVal), pcom);
    }

    return nullptr;
}

EngineQuery SearchStore::constructContainsQuery(const QByteArray& prefix, const QString& value)
{
    QueryParser parser;
    return parser.parseQuery(value, prefix);
}

EngineQuery SearchStore::constructEqualsQuery(const QByteArray& prefix, const QString& value)
{
    // We use the TermGenerator to normalize the words in the value and to
    // split it into other words. If we split the words, we then add them as a
    // phrase query.
    QStringList terms = TermGenerator::termList(value);

    QVector<EngineQuery> queries;
    int position = 1;
    for (const QString& term : terms) {
        QByteArray arr = prefix + term.toUtf8();
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

EngineQuery SearchStore::constructTypeQuery(const QString& value)
{
    Q_ASSERT(!value.isEmpty());

    KFileMetaData::TypeInfo ti = KFileMetaData::TypeInfo::fromName(value);
    if (ti == KFileMetaData::Type::Empty) {
        qDebug() << "Type" << value << "does not exist";
        return EngineQuery();
    }
    int num = static_cast<int>(ti.type());

    return EngineQuery('T' + QByteArray::number(num));
}

PostingIterator* SearchStore::constructMTimeQuery(Transaction* tr, const QDateTime& dt, Term::Comparator com)
{
    Q_ASSERT(dt.isValid());
    quint32 timet = dt.toTime_t();

    MTimeDB::Comparator mtimeCom;
    if (com == Term::Equal) {
        mtimeCom = MTimeDB::Equal;
        quint32 end = QDateTime(dt.date().addDays(1)).toTime_t() - 1;

        return tr->mTimeRangeIter(timet, end);
    }
    else if (com == Term::GreaterEqual) {
        mtimeCom = MTimeDB::GreaterEqual;
    } else if (com == Term::Greater) {
        timet++;
        mtimeCom = MTimeDB::GreaterEqual;
    } else if (com == Term::LessEqual) {
        mtimeCom = MTimeDB::LessEqual;
    } else if (com == Term::Less) {
        mtimeCom = MTimeDB::LessEqual;
        timet--;
    } else {
        Q_ASSERT_X(0, "SearchStore::constructQuery", "mtime query must contain a valid comparator");
        return nullptr;
    }

    return tr->mTimeIter(timet, mtimeCom);
}
