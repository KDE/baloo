/*
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
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
#include "database.h"
#include "fileindexerconfig.h"

#include <QMimeDatabase>
#include <QTimer>
#include <QFile>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>

using namespace Baloo;

Cleaner::Cleaner(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
    QTimer::singleShot(0, this, SLOT(start()));
}

void Cleaner::start()
{
    // FIXME: The cleaner needs to be fixed!!
    /*
    if (!query.exec(QLatin1String("select id, url from files"))) {
        qCDebug(BALOO) << "Could not execute query:" << query.lastError().text();
        QCoreApplication::instance()->quit();
        return;
    }

    FileIndexerConfig config;
    QMimeDatabase mimeDb;

    int numDocuments = 0;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString url = query.value(1).toString();

        bool removeIt = false;
        if (!QFile::exists(url)) {
            removeIt = true;
        }

        if (!config.shouldBeIndexed(url)) {
            removeIt = true;
        }

        // vHanda FIXME: Perhaps we want to get the proper mimetype from xapian?
        QString mimetype = mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchExtension).name();
        if (!config.shouldMimeTypeBeIndexed(mimetype)) {
            removeIt = true;
        }

        if (removeIt) {
            qCDebug(BALOO) << id << url;
            q.prepare(QLatin1String("delete from files where id = ?"));
            q.addBindValue(id);
            q.exec();
            m_commitQueue->remove(id);

            ++numDocuments;
        }

        if (numDocuments && numDocuments % 1000 == 0) {
            m_commitQueue->commit();
        }
    }

    m_commitQueue->commit();
    QCoreApplication::instance()->quit();
    */
}

