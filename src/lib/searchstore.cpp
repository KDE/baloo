/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "baloodebug.h"
#include "searchstore.h"
#include "global.h"

#include "database.h"
#include "enginequery.h"
#include "filenamequery.h"
#include "mimetypequery.h"
#include "propertyphrasequery.h"
#include "query.h"
#include "queryparser.h"
#include "term.h"
#include "termgenerator.h"
#include "transaction.h"

#include "andpostingiterator.h"
#include "orpostingiterator.h"

#include <QDateTime>

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>
#include <KFileMetaData/Types>

#include <algorithm>
#include <array>
#include <limits>
#include <tuple>

namespace Baloo {

using namespace Qt::StringLiterals;

namespace {
QPair<quint32, quint32> calculateTimeRange(const QString &value, Term::Comparator com)
{
    const QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
    if (!dt.isValid()) {
        // Do not warn, input may not be complete
        qCDebug(BALOO) << "Failed to parse timestamp" << value;
        return {1, 0};
    }

    if ((com == Term::Equal) || (com == Term::Contains)) {
        // Timestamps in DB are quint32 relative to Epoch (1970...2106)
        static constexpr qint64 maxUInt32 = std::numeric_limits<quint32>::max();
        quint32 start = std::clamp<qint64>(dt.date().startOfDay().toSecsSinceEpoch(), 0, maxUInt32);
        quint32 end = std::clamp<qint64>(dt.date().endOfDay().toSecsSinceEpoch(), 0, maxUInt32);
        return {start, end};
    }

    quint32 timet = std::clamp<qint64>(dt.toSecsSinceEpoch(), 0, std::numeric_limits<quint32>::max());
    qCDebug(BALOO) << "Parsed/clamped timestamp:" << dt << "->" << timet;

    if (com == Term::LessEqual) {
        return {0, timet};
    }
    if (com == Term::Less) {
        return {0, timet > 0 ? timet - 1 : 0};
    }
    if (com == Term::GreaterEqual) {
        return {timet, std::numeric_limits<quint32>::max()};
    }
    if (com == Term::Greater) {
        return {timet + 1, std::numeric_limits<quint32>::max()};
    }

    Q_ASSERT_X(0, __func__, "mtime query must contain a valid comparator");
    return {1, 0};
}

struct InternalProperty {
    const char* propertyName;
    const char* prefix;
    QMetaType::Type valueType;
};
constexpr std::array<InternalProperty, 4> internalProperties{{
    {"content", "", QMetaType::QString},
    {"rating", "R", QMetaType::Int},
    {"tags", "TA", QMetaType::QString},
    {"usercomment", "C", QMetaType::QString},
}};

std::tuple<bool, QByteArray, QMetaType::Type> propertyInfo(const QByteArray &property)
{
    auto it = std::find_if(std::begin(internalProperties), std::end(internalProperties),
        [&property] (const InternalProperty& entry) { return property == entry.propertyName; });
    if (it != std::end(internalProperties)) {
        return {false, (*it).prefix, (*it).valueType};
    } else {
        KFileMetaData::PropertyInfo pi = KFileMetaData::PropertyInfo::fromName(QString::fromUtf8(property));
        if (pi.property() == KFileMetaData::Property::Empty) {
            return {false, QByteArray(), QMetaType::UnknownType};
        }
        int propPrefix = static_cast<int>(pi.property());
        return {true, QByteArray::number(propPrefix), pi.valueType()};
    }
}

PostingIterator::Ptr constructEqualsQuery(Transaction *tr, const QByteArray &prefix, const QString &value, bool expandLast = false)
{
    // We use the TermGenerator to normalize the words in the value and to
    // split it into other words. If we split the words, we then add them as a
    // phrase query.
    const QByteArrayList terms = TermGenerator::termList(value);
    if (terms.isEmpty()) {
        return nullptr;
    } else if (terms.size() == 1) {
        QByteArray arr = prefix + terms[0];
        if (expandLast && terms[0].size() >= 3) {
            return tr->postingPrefixIterator(arr);
        }
        return tr->postingIterator(arr);
    }

    QVector<EngineQuery::PhraseTerm> queries;
    queries.reserve(terms.size());
    for (const QByteArray& term : terms) {
        QByteArray arr = prefix + term;
        queries.emplaceBack(arr, EngineQuery::Operation::Equal);
    }

    EngineQuery q(queries);
    return tr->postingIterator(q);
}

PostingIterator::Ptr constructContainsQuery(Transaction *tr, const QByteArray &prefix, const QString &value)
{
    return constructEqualsQuery(tr, prefix, value, true);
}

PostingIterator::Ptr constructFolderQuery(Transaction *tr, const QString &value)
{
    const QByteArray folder = value.toUtf8();

    if (folder.isEmpty()) {
        return nullptr;
    }
    if (!folder.startsWith('/')) {
        qCDebug(BALOO) << "Invalid include folder:" << value;
        return nullptr;
    }

    quint64 id = tr->documentId(folder);
    if (!id) {
        qCDebug(BALOO) << "Folder" << value << "not indexed";
        return nullptr;
    }

    return tr->docUrlIter(id);
}

PostingIterator::Ptr constructTypeQuery(Transaction *tr, const QString &value)
{
    Q_ASSERT(!value.isEmpty());

    KFileMetaData::TypeInfo ti = KFileMetaData::TypeInfo::fromName(value);
    if (ti == KFileMetaData::Type::Empty) {
        qCDebug(BALOO) << "Type" << value << "does not exist";
        return {};
    }
    int num = static_cast<int>(ti.type());

    return tr->postingIterator('T' + QByteArray::number(num));
}

QString reconstructFromTerm(const Term &term)
{
    auto comp = term.comparator();
    auto compString = comp == Term::Equal ? u"="_sv //
        : comp == Term::LessEqual         ? u"<="_sv
        : comp == Term::GreaterEqual      ? u">="_sv
        : comp == Term::Less              ? u"<"_sv
        : comp == Term::Greater           ? u">"_sv
                                          : u":"_sv;
    // Reconstruct something like "http://www.kde.org"
    return term.property() + compString + term.value();
}

PostingIterator::Ptr constructQuery(Transaction *tr, const Term &term);

PostingIterator::Ptr constructFromQuery(Transaction *tr, const Query &q)
{
    std::vector<std::unique_ptr<PostingIterator>> vec;

    if (!q.searchString().isEmpty()) {
        Term term = QueryParser::parse(q.searchString());
        if (auto it = constructQuery(tr, term)) {
            vec.push_back(std::move(it));
        } else {
            return {};
        }
    }

    for (const QString &type : q.types()) {
        if (auto it = constructTypeQuery(tr, type)) {
            vec.push_back(std::move(it));
        } else {
            return {};
        }
    }

    if (!q.includeFolder().isEmpty()) {
        if (auto it = constructFolderQuery(tr, q.includeFolder())) {
            vec.push_back(std::move(it));
        } else {
            return {};
        }
    }

    if (q.yearFilter() || q.monthFilter() || q.dayFilter()) {
        int year = q.yearFilter();
        int month = q.monthFilter();
        int day = q.dayFilter();

        month = month >= 0 && month <= 12 ? month : 0;
        day = day >= 0 && day <= 31 ? day : 0;

        QDate startDate(year, month ? month : 1, day ? day : 1);
        QDate endDate(startDate);

        if (month == 0) {
            endDate.setDate(endDate.year(), 12, 31);
        } else if (day == 0) {
            endDate.setDate(endDate.year(), endDate.month(), endDate.daysInMonth());
        }

        auto start = startDate.startOfDay().toSecsSinceEpoch();
        auto end = endDate.endOfDay().toSecsSinceEpoch();
        if (auto it = tr->mTimeRangeIter(start, end)) {
            vec.push_back(std::move(it));
        } else {
            qCDebug(BALOO) << "Invalid year/month/day filter:" << year << month << day;
            return {};
        }
    }

    return std::make_unique<AndPostingIterator>(std::move(vec));
}
} // namespace

ResultList SearchStore::exec(const Query &query)
{
    auto db = globalDatabaseInstance();
    if (!db || db->open(Database::ReadOnlyDatabase) != Database::OpenResult::Success) {
        return ResultList();
    }

    Transaction tr(db, Transaction::ReadOnly);
    auto it = constructFromQuery(&tr, query);
    if (!it) {
        return ResultList();
    }

    if (query.sortingOption() == Query::SortAuto) {
        std::vector<std::pair<quint64, quint32>> resultIds;
        while (it->next()) {
            quint64 id = it->docId();
            quint32 mtime = tr.documentTimeInfo(id).mTime;
            resultIds.emplace_back(id, mtime);

            Q_ASSERT(id > 0);
        }

        // Not enough results within range, no need to sort.
        const size_t offset = query.offset();
        if (offset >= resultIds.size()) {
            return ResultList();
        }

        auto compFunc = [](const std::pair<quint64, quint32>& lhs,
                           const std::pair<quint64, quint32>& rhs) {
            return lhs.second > rhs.second;
        };

        std::sort(resultIds.begin(), resultIds.end(), compFunc);

        ResultList results;
        const size_t end = [&query, offset, count = resultIds.size()]() {
            if (query.limit() == std::numeric_limits<uint>::max()) {
                return count;
            }
            return std::min<size_t>(offset + query.limit(), count);
        }();

        results.reserve(end - offset);
        for (size_t i = offset; i < end; i++) {
            const quint64 id = resultIds[i].first;
            Result res{tr.documentUrl(id), id};

            results.emplace_back(res);
        }

        return results;

    } else {
        ResultList results;
        uint ulimit = query.limit();
        uint offset = query.offset();

        while (offset && it->next()) {
            offset--;
        }

        while (ulimit && it->next()) {
            const quint64 id = it->docId();
            Q_ASSERT(id > 0);
            Result res{tr.documentUrl(id), id};
            Q_ASSERT(!res.filePath.isEmpty());

            results.emplace_back(res);

            ulimit--;
        }

        return results;
    }
}

namespace
{
PostingIterator::Ptr constructQuery(Transaction *tr, const Term &term)
{
    Q_ASSERT(tr);

    if (term.operation() == Term::And || term.operation() == Term::Or) {
        const QList<Term> subTerms = term.subTerms();
        std::vector<std::unique_ptr<PostingIterator>> vec;
        vec.reserve(subTerms.size());

        for (const Term& t : subTerms) {
            auto iterator = constructQuery(tr, t);
            // constructQuery returns a nullptr to signal an empty list
            if (iterator) {
                vec.push_back(std::move(iterator));
            } else if (term.operation() == Term::And) {
                return nullptr;
            }
        }

        if (vec.empty()) {
            return nullptr;
        } else if (vec.size() == 1) {
            return std::move(vec.front());
        }

        if (term.operation() == Term::And) {
            return std::make_unique<AndPostingIterator>(std::move(vec));
        } else {
            return std::make_unique<OrPostingIterator>(std::move(vec));
        }
    }

    if (term.value().isNull()) {
        return nullptr;
    }
    Q_ASSERT(!term.isEmpty());
    Q_ASSERT(term.comparator() != Term::Auto);

    const QString value = term.value();
    const QByteArray property = term.property().toLower().toUtf8();

    if (property == "type" || property == "kind") {
        return constructTypeQuery(tr, value);
    }
    else if (property == "includefolder") {
        return constructFolderQuery(tr, value);
    }
    else if (property == "modified" || property == "mtime") {
        QPair<quint32, quint32> timerange = calculateTimeRange(value, term.comparator());
        if (timerange.first > timerange.second) {
            return nullptr;
        }
        return tr->mTimeRangeIter(timerange.first, timerange.second);
    } else if (property == "tag") {
        if (term.comparator() == Term::Equal) {
            const QByteArray prefix = "TAG-";
            return tr->postingIterator(prefix + value.toUtf8());
        } else {
            // Delegate to "internalProperties" "tags" handler
            return constructQuery(tr, Term{u"tags"_s, term.value(), term.comparator()});
        }
    } else if (property == "") {
        Term cterm(QStringLiteral("content"), term.value(), term.comparator());
        Term fterm(QStringLiteral("filename"), term.value(), term.comparator());
        return constructQuery(tr, Term{cterm, Term::Operation::Or, fterm});
    } else if (property == "filename") {
        auto com = term.comparator();
        auto fq = FilenameQuery(value, (com == Term::Contains));
        qCDebug(BALOO) << "filename query:" << fq;
        return tr->postingIterator(fq);
    } else if (property == "mimetype") {
        auto com = term.comparator();
        auto mq = MimetypeQuery(value, (com == Term::Contains));
        qCDebug(BALOO) << "mimetype query:" << mq;
        return tr->postingIterator(mq);
    }

    bool isProperty = false;
    QByteArray prefix;
    QMetaType::Type valueType = QMetaType::QString;
    if (!property.isEmpty()) {
        std::tie(isProperty, prefix, valueType) = propertyInfo(property);
        if (valueType == QMetaType::UnknownType) {
            QString reconstructed{reconstructFromTerm(term)};
            qCDebug(BALOO) << "Unknown property" << property << "with value" //
                           << term.value() << "- using" << reconstructed;
            return constructQuery(tr, Term{QString{}, reconstructed, Term::Equal});
        }
    }
    if (isProperty && valueType == QMetaType::QString) {
        auto com = term.comparator();
        auto pq = PropertyPhraseQuery(value, prefix, (com == Term::Contains));
        qCDebug(BALOO) << "property query:" << pq;
        return tr->postingIterator(pq);
    } else if (isProperty) {
        prefix = 'X' + prefix + '-';
    }

    auto com = term.comparator();
    if (com == Term::Contains && valueType == QMetaType::Int) {
        com = Term::Equal;
    }
    if (com == Term::Contains) {
        return constructContainsQuery(tr, prefix, value);
    }

    if (com == Term::Equal) {
        return constructEqualsQuery(tr, prefix, value);
    }

    PostingDB::Comparator pcom;
    if (com == Term::Greater || com == Term::GreaterEqual) {
        pcom = PostingDB::GreaterEqual;
    } else if (com == Term::Less || com == Term::LessEqual) {
        pcom = PostingDB::LessEqual;
    }

    // FIXME -- has to be kept in sync with the code from
    // Baloo::Result::add
    if (valueType == QMetaType::Int) {
        bool ok = false;
        qlonglong intVal = value.toLongLong(&ok);
        if (!ok) {
            qCDebug(BALOO) << "Failed to parse number" << value;
            return nullptr;
        }

        if ((term.comparator() == Term::Greater) && (intVal < std::numeric_limits<qlonglong>::max())) {
            intVal++;
        } else if ((term.comparator() == Term::Less) && (intVal > std::numeric_limits<qlonglong>::min())) {
            intVal--;
        }
        return tr->postingCompIterator(prefix, intVal, pcom);

    } else if (valueType == QMetaType::Double) {
        bool ok = false;
        double dVal = value.toDouble(&ok);
        if (!ok) {
            qCDebug(BALOO) << "Failed to parse number" << value;
            return nullptr;
        }
        return tr->postingCompIterator(prefix, dVal, pcom);

    } else if (valueType == QMetaType::QDateTime) {
        const QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
        if (!dt.isValid()) {
            qCDebug(BALOO) << "Failed to parse date/time" << value;
            return nullptr;
        }
        const QByteArray ba = dt.toString(Qt::ISODate).toUtf8();
        return tr->postingCompIterator(prefix, ba, pcom);
    }

    qCWarning(BALOO) << "Invalid operator" << ((com == Term::LessEqual) || (com == Term::Less) ? "'<'" : "'>'") << "for string property";

    return nullptr;
}
} // namepspace <anonymous>

} // namespace Baloo
