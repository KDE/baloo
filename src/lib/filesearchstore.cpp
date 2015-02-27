/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2014 Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of vCMakeFiles/KF5Baloo.dir/filesearchstore.cpp.o: In function `Baloo::LuceneSearchStore::~LuceneSearchStore()':ersion 3 of the license.
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

#include "filesearchstore.h"
#include "term.h"
#include "filemapping.h"
#include "xapiantermgenerator.h"

#include <QVector>
#include <QDate>
#include <QUrl>
#include <QStandardPaths>
#include <climits>

#include <QDebug>
#include <QMimeDatabase>

#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

FileSearchStore::FileSearchStore(QObject* parent)
    : LuceneSearchStore(parent)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file/");
    setIndexPath(path);

    m_prefixes.insert(QStringLiteral("filename"), QStringLiteral("F"));
    m_prefixes.insert(QStringLiteral("mimetype"), QStringLiteral("M"));
    m_prefixes.insert(QStringLiteral("rating"), QStringLiteral("R"));
    m_prefixes.insert(QStringLiteral("tag"), QStringLiteral("TA"));
    m_prefixes.insert(QStringLiteral("tags"), QStringLiteral("TA"));
    m_prefixes.insert(QStringLiteral("usercomment"), QStringLiteral("C"));
    m_prefixes.insert(QStringLiteral("type"), QStringLiteral("T"));
    m_prefixes.insert(QStringLiteral("kind"), QStringLiteral("T"));
}

FileSearchStore::~FileSearchStore()
{
}

void FileSearchStore::setIndexPath(const QString& path)
{
    Baloo::LuceneSearchStore::setIndexPath(path);
}

QStringList FileSearchStore::types()
{
    return QStringList() << QLatin1String("File") << QLatin1String("Audio") << QLatin1String("Video") << QLatin1String("Document") << QLatin1String("Image") << QLatin1String("Archive") << QLatin1String("Folder");
}

Lucene::QueryPtr FileSearchStore::convertTypes(const QStringList& types)
{
    Lucene::BooleanQueryPtr boolQuery = Lucene::newLucene<Lucene::BooleanQuery>();
    Q_FOREACH (const QString& type, types) {
        if (type == QLatin1String("file"))
            continue;
        Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(L"T", type.toLower().toStdWString());
        boolQuery->add(Lucene::newLucene<Lucene::TermQuery>(term), Lucene::BooleanClause::MUST);
    }

    return boolQuery->rewrite(m_reader);
}

QString FileSearchStore::fetchPrefix(const QString& property) const
{
    auto it = m_prefixes.constFind(property.toLower());
    if (it != m_prefixes.constEnd()) {
        return it.value();
    }
    else {
        KFileMetaData::PropertyInfo pi = KFileMetaData::PropertyInfo::fromName(property);
        if (pi.property() == KFileMetaData::Property::Empty) {
            qDebug() << "Property" << property << "not found";
            return QString();
        }
        int propPrefix = static_cast<int>(pi.property());
        return QLatin1Char('X') + QString::number(propPrefix);
    }
}

Lucene::QueryPtr FileSearchStore::constructQuery(const QString& property, const QVariant& value,
                                              Term::Comparator com)
{
    // FIXME: Handle cases where only the property is specified
    Lucene::QueryPtr emptyQuery = Lucene::newLucene<Lucene::BooleanQuery>();
    if (value.isNull()) {
        return emptyQuery;
    }
    //
    // Sanitize the values
    //
    if (value.type() == QVariant::Int && com == Term::Contains) {
        com = Term::Equal;
    }
    if (property.compare(QLatin1String("rating"), Qt::CaseInsensitive) == 0) {
        int val = value.toInt();
        if (val == 0)
            return emptyQuery;
        Lucene::NumericRangeQueryPtr rangeQuery;
        switch (com) {
            case Term::GreaterEqual :
            case Term::Greater :
                rangeQuery = Lucene::NumericRangeQuery::newIntRange(L"R", val, 10, com == Term::GreaterEqual, true);
                break;
            case Term::LessEqual :
            case Term::Less :
                rangeQuery = Lucene::NumericRangeQuery::newIntRange(L"R", 1, val, true, com == Term::LessEqual);
                break;
            default:
                break;
        }
        return rangeQuery->rewrite(m_reader);
    }

    if (property == QStringLiteral("filename") && value.toString().contains('*')) {
        Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(L"F", value.toString().toStdWString());
        Lucene::WildcardQueryPtr wildCardQuery = Lucene::newLucene<Lucene::WildcardQuery>(term);
        return wildCardQuery->rewrite(m_reader);
    }

    if (property.compare(QLatin1String("modified"), Qt::CaseInsensitive) == 0) {
        if (com == Term::Equal || com == Term::Contains) {

            QString val;
            if (value.type() == QVariant::Date) {
                val = value.toDate().toString(Qt::ISODate);
            } else if (value.type() == QVariant::DateTime) {
                val = value.toDateTime().toString(Qt::ISODate);
            }
            Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(L"DT_M", val.toStdWString());
            Lucene::PrefixQueryPtr preQuery = Lucene::newLucene<Lucene::PrefixQuery>(term);
            return preQuery->rewrite(m_reader);
        }

        if (com == Term::Greater || com == Term::GreaterEqual ||
            com == Term::Less || com == Term::LessEqual)
        {
            qlonglong numVal = 0;
            QString field;
            // we intialize max to  max value of long, just in case we can't get current time to limit queries
            //TODO check if lucene's implicit cast to long causes loss of data??
            qint64 max = LONG_MAX;

            if (value.type() == QVariant::DateTime) {
                field = "time_t";
                numVal = value.toDateTime().toTime_t();
                max = QDateTime::currentDateTime().toTime_t();
            }
            else if (value.type() == QVariant::Date) {
                field = "j_day";
                numVal = value.toDate().toJulianDay();
                max = QDate::currentDate().toJulianDay();
            }

            Lucene::NumericRangeQueryPtr rangeQuery;
            switch (com) {
                case Term::GreaterEqual :
                case Term::Greater :
                    rangeQuery = Lucene::NumericRangeQuery::newLongRange(field.toStdWString(), numVal, max, com == Term::GreaterEqual, true);
                    break;
                case Term::LessEqual :
                case Term::Less :
                    rangeQuery = Lucene::NumericRangeQuery::newLongRange(field.toStdWString(), 0, numVal, false, com == Term::LessEqual);
                default :
                    break;
            }
            return rangeQuery->rewrite(m_reader);
        }
    }

    if (value.type() == QVariant::Date || value.type() == QVariant::DateTime) {
        if (com == Term::Equal || com == Term::Contains) {

            QString val;
            if (value.type() == QVariant::Date) {
                val = value.toDate().toString(Qt::ISODate);
            } else if (value.type() == QVariant::DateTime) {
                val = value.toDateTime().toString(Qt::ISODate);
            }
            Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(fetchPrefix(property).toStdWString(), val.toStdWString());
            Lucene::PrefixQueryPtr preQuery = Lucene::newLucene<Lucene::PrefixQuery>(term);
            return preQuery->rewrite(m_reader);
        }
        // FIXME: Should we expressly forbid other comparisons?
    }

    if (com == Term::Contains) {
        QString prefix;
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                Lucene::QueryPtr emptyQuery = Lucene::newLucene<Lucene::BooleanQuery>();
                return emptyQuery;
            }
        }
        Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(prefix.toStdWString(), value.toString().toStdWString());
        Lucene::PrefixQueryPtr preQuery = Lucene::newLucene<Lucene::PrefixQuery>(term);
        return preQuery->rewrite(m_reader);
    }

    if (com == Term::Equal) {
        // We use the TermGenerator to normalize the words in the value and to
        // split it into other words. If we split the words, we then add them as a
        // phrase query.
        QStringList terms = XapianTermGenerator::termList(value.toString());

        // If no property is specified we default queries to use content field
        QString prefix = "content";
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                Lucene::QueryPtr emptyQuery = Lucene::newLucene<Lucene::BooleanQuery>();
                return emptyQuery;
            }
        }


        Lucene::PhraseQueryPtr phraseQ = Lucene::newLucene<Lucene::PhraseQuery>();
        for (const QString& term : terms) {
            Lucene::TermPtr luceneTerm = Lucene::newLucene<Lucene::Term>(prefix.toStdWString(), term.toStdWString());
            phraseQ->add(luceneTerm);
        }
        return phraseQ->rewrite(m_reader);
    }
    return Lucene::newLucene<Lucene::BooleanQuery>();
}

Lucene::QueryPtr FileSearchStore::constructFilterQuery(int year, int month, int day)
{
    Lucene::BooleanQueryPtr boolQuery = Lucene::newLucene<Lucene::BooleanQuery>();

    if (year != -1) {
        // we need to use range queries with same start and end when matching exact values for numeric fields
        Lucene::NumericRangeQueryPtr rangeQ = Lucene::NumericRangeQuery::newIntRange(L"DT_MY", year, year, true, true);
        boolQuery->add(rangeQ->rewrite(m_reader), Lucene::BooleanClause::MUST);
    }
    if (month != -1) {
        Lucene::NumericRangeQueryPtr rangeQ = Lucene::NumericRangeQuery::newIntRange(L"DT_MM", month, month, true, true);
        boolQuery->add(rangeQ->rewrite(m_reader), Lucene::BooleanClause::MUST);
    }
    if (day != -1) {
        Lucene::NumericRangeQueryPtr rangeQ = Lucene::NumericRangeQuery::newIntRange(L"DT_MD", day, day, true, true);
        boolQuery->add(rangeQ->rewrite(m_reader), Lucene::BooleanClause::MUST);
    }

    return boolQuery->rewrite(m_reader);
}

QString FileSearchStore::constructFilePath(int docid)
{
    QMutexLocker lock(&m_mutex);

    FileMapping file(docid);
    file.fetch(m_reader);

    return file.url();
}

Lucene::QueryPtr FileSearchStore::applyIncludeFolder(const Lucene::QueryPtr& q, QString includeFolder)
{
    if (includeFolder.isEmpty()) {
        return q;
    }
    if (!includeFolder.endsWith(QLatin1Char('/'))) {
        includeFolder.append(QLatin1Char('/'));
    }
    Lucene::PrefixQueryPtr urlQuery = Lucene::newLucene<Lucene::PrefixQuery>(
        Lucene::newLucene<Lucene::Term>(L"URL", includeFolder.toStdWString()) );
    return andQuery(q, urlQuery->rewrite(m_reader));
}
