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

#include "filesearchstore.h"
#include "term.h"
#include "query.h"
#include "filemapping.h"
#include "pathfilterpostingsource.h"
#include "wildcardpostingsource.h"
#include "xapianqueryparser.h"
#include "xapiantermgenerator.h"

#include <xapian.h>
#include <QVector>
#include <QDate>
#include <QUrl>
#include <QStandardPaths>

#include <QDebug>
#include <QMimeDatabase>

#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

FileSearchStore::FileSearchStore(QObject* parent)
    : XapianSearchStore(parent)
    , m_sqlMutex(QMutex::Recursive)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file/");
    setDbPath(path);

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
    const QString conName = m_sqlDb.connectionName();
    m_sqlDb.close();
    m_sqlDb = QSqlDatabase();

    QSqlDatabase::removeDatabase(conName);
}

void FileSearchStore::setDbPath(const QString& path)
{
    XapianSearchStore::setDbPath(path);

    const QString conName = QLatin1String("filesearchstore") + QString::number(qrand());

    m_sqlDb = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), conName);
    m_sqlDb.setDatabaseName(dbPath() + QLatin1String("/fileMap.sqlite3"));
    m_sqlDb.open();
}

QStringList FileSearchStore::types()
{
    return QStringList() << QLatin1String("File") << QLatin1String("Audio") << QLatin1String("Video") << QLatin1String("Document") << QLatin1String("Image") << QLatin1String("Archive") << QLatin1String("Folder");
}

Xapian::Query FileSearchStore::convertTypes(const QStringList& types)
{
    Xapian::Query xapQ;
    Q_FOREACH (const QString& type, types) {
        const QString t = QLatin1Char('T') + type.toLower();
        if (t == QLatin1String("Tfile"))
            continue;

        const QByteArray arr = t.toUtf8();
        xapQ = andQuery(xapQ, Xapian::Query(arr.constData()));
    }

    return xapQ;
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

Xapian::Query FileSearchStore::constructQuery(const QString& property, const QVariant& value,
                                              Term::Comparator com)
{
    // FIXME: Handle cases where only the property is specified
    if (value.isNull())
        return Xapian::Query();

    //
    // Sanitize the values
    //
    if (value.type() == QVariant::Int && com == Term::Contains) {
        com = Term::Equal;
    }

    if (property.compare(QLatin1String("rating"), Qt::CaseInsensitive) == 0) {
        int val = value.toInt();
        if (val == 0)
            return Xapian::Query();

        QVector<std::string> terms;
        if (com == Term::Greater || com == Term::GreaterEqual) {
            if (com == Term::Greater)
                val++;

            for (int i=val; i<=10; ++i) {
                QByteArray arr = 'R' + QByteArray::number(i);
                terms << arr.constData();
            }
        }
        else if (com == Term::Less || com == Term::LessEqual) {
            if (com == Term::Less)
                val--;

            for (int i=1; i<=val; ++i) {
                QByteArray arr = 'R' + QByteArray::number(i);
                terms << arr.constData();
            }
        }
        else if (com == Term::Equal) {
            QByteArray arr = 'R' + QByteArray::number(val);
            terms << arr.constData();
        }

        return Xapian::Query(Xapian::Query::OP_OR, terms.begin(), terms.end());
    }

    if (property == QStringLiteral("filename") && value.toString().contains('*')) {
        WildcardPostingSource ws(value.toString(), QStringLiteral("F"));
        return Xapian::Query(&ws);
    }

    if (property.compare(QLatin1String("modified"), Qt::CaseInsensitive) == 0) {
        if (com == Term::Equal || com == Term::Contains) {
            XapianQueryParser parser;
            parser.setDatabase(xapianDb());

            QString val;
            if (value.type() == QVariant::Date) {
                val = value.toDate().toString(Qt::ISODate);
            } else if (value.type() == QVariant::DateTime) {
                val = value.toDateTime().toString(Qt::ISODate);
            }

            return parser.expandWord(val, QStringLiteral("DT_M"));
        }

        if (com == Term::Greater || com == Term::GreaterEqual ||
            com == Term::Less || com == Term::LessEqual)
        {
            qlonglong numVal = 0;
            int slotNumber = 0;

            if (value.type() == QVariant::DateTime) {
                slotNumber = 0;
                numVal = value.toDateTime().toTime_t();
            }
            else if (value.type() == QVariant::Date) {
                slotNumber = 1;
                numVal = value.toDate().toJulianDay();
            }

            if (com == Term::Greater) {
                ++numVal;
            }
            if (com == Term::Less) {
                --numVal;
            }

            if (com == Term::GreaterEqual || com == Term::Greater) {
                return Xapian::Query(Xapian::Query::OP_VALUE_GE, slotNumber, QString::number(numVal).toStdString());
            }
            else if (com == Term::LessEqual || com == Term::Less) {
                return Xapian::Query(Xapian::Query::OP_VALUE_LE, slotNumber, QString::number(numVal).toStdString());
            }
        }
    }

    if (value.type() == QVariant::Date || value.type() == QVariant::DateTime) {
        if (com == Term::Equal || com == Term::Contains) {
            XapianQueryParser parser;
            parser.setDatabase(xapianDb());

            QString val;
            if (value.type() == QVariant::Date) {
                val = value.toDate().toString(Qt::ISODate);
            } else if (value.type() == QVariant::DateTime) {
                val = value.toDateTime().toString(Qt::ISODate);
            }

            return parser.expandWord(val, fetchPrefix(property));
        }
        // FIXME: Should we expressly forbid other comparisons?
    }

    if (com == Term::Contains) {
        XapianQueryParser parser;
        parser.setDatabase(xapianDb());

        QString prefix;
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                return Xapian::Query();
            }
        }

        return parser.parseQuery(value.toString(), prefix);
    }

    if (com == Term::Equal) {
        // We use the TermGenerator to normalize the words in the value and to
        // split it into other words. If we split the words, we then add them as a
        // phrase query.
        QStringList terms = XapianTermGenerator::termList(value.toString());

        QString prefix;
        if (!property.isEmpty()) {
            prefix = fetchPrefix(property);
            if (prefix.isEmpty()) {
                return Xapian::Query();
            }
        }

        QList<Xapian::Query> queries;
        int position = 1;
        for (const QString& term : terms) {
            QByteArray arr = (prefix + term).toUtf8();
            queries << Xapian::Query(arr.constData(), 1, position++);
        }

        if (queries.isEmpty()) {
            return Xapian::Query();
        } else if (queries.size() == 1) {
            return queries.first();
        } else {
            return Xapian::Query(Xapian::Query::OP_PHRASE, queries.begin(), queries.end());
        }
    }

    return Xapian::Query();
}

Xapian::Query FileSearchStore::constructFilterQuery(int year, int month, int day)
{
    QVector<std::string> vector;
    vector.reserve(3);

    if (year != -1)
        vector << QString::fromLatin1("DT_MY%1").arg(year).toUtf8().constData();
    if (month != -1)
        vector << QString::fromLatin1("DT_MM%1").arg(month).toUtf8().constData();
    if (day != -1)
        vector << QString::fromLatin1("DT_MD%1").arg(day).toUtf8().constData();

    return Xapian::Query(Xapian::Query::OP_AND, vector.begin(), vector.end());
}

QString FileSearchStore::constructFilePath(const Xapian::docid& docid)
{
    QMutexLocker lock(&m_sqlMutex);

    FileMapping file(docid);
    file.fetch(m_sqlDb);

    return file.url();
}

Xapian::Query FileSearchStore::applyIncludeFolder(const Xapian::Query& q, const QString& includeFolder)
{
    if (includeFolder.isEmpty()) {
        return q;
    }

    PathFilterPostingSource ps(&m_sqlDb, includeFolder);
    return andQuery(q, Xapian::Query(&ps));
}
