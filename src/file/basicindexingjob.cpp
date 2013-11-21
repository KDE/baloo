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

BasicIndexingJob::BasicIndexingJob(Database* m_db, const FileMapping& file,
                                   const QString& mimetype, QObject* parent)
    : KJob(parent)
    , m_db(m_db)
    , m_file(file)
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
    if (m_file.id() == 0) {
        if (!m_file.create(m_db->sqlDatabase())) {
            setError(1);
            setErrorText("Cannot create fileMapping for" + QString::number(m_file.id()));
            emitResult();
            return;
        }
    }

    QFileInfo fileInfo(m_file.url());

    // FIXME: We will need a termGenerator
    // FIXME: Fetch the type?

    Xapian::Document doc;
    doc.add_term('M' + m_mimetype.toStdString());
    doc.add_term('F' + fileInfo.fileName().toStdString());

    // Modified Date
    QDateTime mod = fileInfo.lastModified();
    doc.add_term("DT_M" + fileInfo.lastModified().toString(Qt::ISODate).toStdString());

    const QString year = "DT_MY" + QString::number(mod.date().year());
    const QString month = "DT_MM" + QString::number(mod.date().month());
    const QString day = "DT_MD" + QString::number(mod.date().day());
    doc.add_term(year.toStdString());
    doc.add_term(month.toStdString());
    doc.add_term(day.toStdString());

    // Indexing Level 1
    doc.add_term("Z1");

    m_db->xapainDatabase()->replace_document(m_file.id(), doc);

    emitResult();
}
