/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "cleaner.h"
#include "../database.h"
#include "../fileindexerconfig.h"

#include <KMimeType>

#include <QTimer>
#include <QFile>
#include <QUrl>
#include <QVariant>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>

using namespace Baloo;

Cleaner::Cleaner(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
    m_commitQueue = new CommitQueue(m_db, this);

    QTimer::singleShot(0, this, SLOT(start()));
}

void Cleaner::start()
{
    QSqlDatabase sqlDb = m_db->sqlDatabase();
    QSqlQuery query(sqlDb);
    query.exec(QLatin1String("select id, url from files"));

    FileIndexerConfig config;

    while (query.next()) {
        int id = query.value(0).toInt();
        QString url = query.value(1).toString();

        if (!QFile::exists(url)) {
            QSqlQuery q(sqlDb);
            q.prepare("delete from files where id = ?");
            q.addBindValue(id);
            q.exec();
            m_commitQueue->remove(id);
            continue;
        }

        if (!config.shouldBeIndexed(url)) {
            m_commitQueue->remove(id);
            continue;
        }

        QString mimetype = KMimeType::findByUrl(QUrl::fromLocalFile(url))->name();
        if (!config.shouldMimeTypeBeIndexed(mimetype)) {
            m_commitQueue->remove(id);
            continue;
        }
    }

    m_commitQueue->commit();
    QCoreApplication::instance()->quit();
}

