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

#include <xapian.h>
#include "basicindexingjob.h"

#include <QTimer>
#include <QVariant>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlQuery>

#include <KStandardDirs>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(const QString& url, const QString& mimetype, QObject* parent)
    : KJob(parent)
    , m_url(url)
    , m_mimetype(mimetype)
{
}

BasicIndexingJob::~BasicIndexingJob()
{
}

void BasicIndexingJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void BasicIndexingJob::doStart()
{
    int id = fetchFileId(m_url);

    const QString dbPath = KStandardDirs::locateLocal("data", "baloo/file/");

    Xapian::WritableDatabase db = Xapian::WritableDatabase(dbPath.toStdString(),
                                                           Xapian::DB_CREATE_OR_OPEN);

    QFileInfo fileInfo(m_url);

    Xapian::Document doc;
    doc.add_term('M' + m_mimetype.toStdString());
    // FIXME: We will need a termGenerator
    doc.add_term('F' + fileInfo.fileName().toStdString());
    doc.add_term("DT_M" + fileInfo.lastModified().toString(Qt::ISODate).toStdString());
    // FIXME: Fetch the type?

    db.replace_document(id, doc);
    db.commit();

    emitResult();
}

int BasicIndexingJob::fetchFileId(const QString& url)
{
    QSqlQuery query;
    query.prepare(QLatin1String("insert into files (url) VALUES (?)"));
    query.addBindValue(url);
    query.exec();

    return query.lastInsertId().toInt();
}

