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

#include "filesearchstore.h"
#include "item.h"
#include "term.h"
#include "query.h"
#include "filemapping.h"
#include "pathfilterpostingsource.h"

#include <xapian.h>
#include <QVector>

#include <KStandardDirs>
#include <KDebug>
#include <KUrl>

using namespace Baloo;

FileSearchStore::FileSearchStore(QObject* parent, const QVariantList&)
    : XapianSearchStore(parent)
    , m_sqlDb(0)
    , m_sqlMutex(QMutex::Recursive)
{
    const QString path = KStandardDirs::locateLocal("data", "baloo/file/");
    setDbPath(path);

    m_prefixes.insert("filename", "F");
    m_prefixes.insert("mimetype", "M");
    m_prefixes.insert("rating", "R");
    m_prefixes.insert("tag", "TAG");
    m_prefixes.insert("tags", "TAG");
    m_prefixes.insert("usercomment", "C");
}

FileSearchStore::~FileSearchStore()
{
    const QString conName = m_sqlDb->connectionName();
    delete m_sqlDb;
    QSqlDatabase::removeDatabase(conName);
}

void FileSearchStore::setDbPath(const QString& path)
{
    XapianSearchStore::setDbPath(path);

    const QString conName = "filesearchstore" + QString::number(qrand());

    delete m_sqlDb;
    m_sqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE3", conName));
    m_sqlDb->setDatabaseName(dbPath() + "/fileMap.sqlite3");
    m_sqlDb->open();
}

QStringList FileSearchStore::types()
{
    return QStringList() << "File" << "Audio" << "Video" << "Document" << "Image" << "Archive";
}

Xapian::Query FileSearchStore::convertTypes(const QStringList& types)
{
    Xapian::Query xapQ;
    Q_FOREACH (const QString& type, types) {
        QString t = 'T' + type.toLower();
        if (t == "Tfile")
            continue;

        xapQ = andQuery(xapQ, Xapian::Query(t.toStdString()));
    }

    return xapQ;
}

Xapian::Query FileSearchStore::constructQuery(const QString& property, const QVariant& value,
                                              Term::Comparator com)
{
    // FIXME: Handle cases where only the property is specified
    if (value.isNull())
        return Xapian::Query();

    if (property.compare(QLatin1String("rating"), Qt::CaseInsensitive) == 0) {
        int val = value.toInt();
        if (val == 0)
            return Xapian::Query();

        QVector<std::string> terms;
        if (com == Term::Greater || com == Term::GreaterEqual) {
            if (com == Term::Greater)
                val++;

            for (int i=val; i<=10; i++) {
                QByteArray arr = 'R' + QByteArray::number(i);
                terms << arr.constData();
            }
        }
        else if (com == Term::Less || com == Term::LessEqual) {
            if (com == Term::Less)
                val--;

            for (int i=1; i<=val; i++) {
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

    if (com == Term::Contains) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p;
        QHash<QString, std::string>::const_iterator it = m_prefixes.constFind(property.toLower());
        if (it != m_prefixes.constEnd())
            p = it.value();
        else
            p = property.toUpper().toStdString();

        std::string str = value.toString().toStdString();
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        return parser.parse_query(str, flags, p);
    }

    return Xapian::Query(value.toString().toStdString());
}

Xapian::Query FileSearchStore::constructFilterQuery(int year, int month, int day)
{
    QVector<std::string> vector;
    vector.reserve(3);

    if (year != -1)
        vector << QString::fromLatin1("DT_MY%1").arg(year).toStdString();
    if (month != -1)
        vector << QString::fromLatin1("DT_MM%1").arg(month).toStdString();
    if (day != -1)
        vector << QString::fromLatin1("DT_MD%1").arg(day).toStdString();

    return Xapian::Query(Xapian::Query::OP_AND, vector.begin(), vector.end());
}

QUrl FileSearchStore::constructUrl(const Xapian::docid& docid)
{
    QMutexLocker lock(&m_sqlMutex);

    FileMapping file(docid);
    file.fetch(*m_sqlDb);

    return QUrl::fromLocalFile(file.url());
}

QString FileSearchStore::text(int queryId)
{
    return KUrl(url(queryId)).fileName();
}

Xapian::Query FileSearchStore::applyCustomOptions(const Xapian::Query& q, const QVariantHash& options)
{
    QHash<QString, QVariant>::const_iterator it = options.constFind("includeFolder");
    if (it == options.constEnd()) {
        return q;
    }

    QString includeDir = it.value().toString();

    PathFilterPostingSource ps(m_sqlDb, includeDir);
    return andQuery(q, Xapian::Query(&ps));
}

BALOO_EXPORT_SEARCHSTORE(Baloo::FileSearchStore, "baloo_filesearchstore")
