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
#include "filemapping.h"

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
    return QStringList() << "File";
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

    if (com == Term::Contains) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p = property.toUpper().toStdString();
        std::string str = value.toString().toStdString();
        int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
        return parser.parse_query(str, flags, p);
    }

    return Xapian::Query(value.toString().toStdString());
}

QUrl FileSearchStore::urlFromDoc(const Xapian::docid& docid)
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


BALOO_EXPORT_SEARCHSTORE(Baloo::FileSearchStore, "baloo_filesearchstore")
