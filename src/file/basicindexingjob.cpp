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

#include "basicindexingjob.h"
#include "database.h"

#include <QTimer>
#include <QVariant>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlQuery>

#include <KDebug>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(Database* m_db, int fileId, const QString& url,
                                   const QString& mimetype, QObject* parent)
    : KJob(parent)
    , m_db(m_db)
    , m_fileId(fileId)
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
    if (m_fileId == 0) {
        m_fileId = createFileId(m_url);
        if (m_fileId == 0) {
            emitResult();
        }
    }

    QFileInfo fileInfo(m_url);

    // FIXME: We will need a termGenerator
    // FIXME: Fetch the type?

    Xapian::Document doc;
    doc.add_term('M' + m_mimetype.toStdString());
    doc.add_term('F' + fileInfo.fileName().toStdString());
    doc.add_term("DT_M" + fileInfo.lastModified().toString(Qt::ISODate).toStdString());

    m_db->xapainDatabase()->replace_document(m_fileId, doc);

    emitResult();
}

int BasicIndexingJob::createFileId(const QString& url)
{
    QSqlQuery query(m_db->sqlDatabase());
    query.prepare(QLatin1String("insert into files (url) VALUES (?)"));
    query.addBindValue(url);
    query.exec();

    return query.lastInsertId().toInt();
}

