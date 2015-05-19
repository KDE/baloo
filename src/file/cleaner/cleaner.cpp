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
#include "transaction.h"
#include "fileindexerconfig.h"
#include "idutils.h"
#include "baloodebug.h"

#include <QMimeDatabase>
#include <QTimer>
#include <QFile>
#include <QCoreApplication>
#include <QDir>

using namespace Baloo;

Cleaner::Cleaner(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
    QTimer::singleShot(0, this, SLOT(start()));
}

void Cleaner::start()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/baloo");

    if (!QFile::exists(path + "/index")) {
        QCoreApplication::exit(0);
        return;
    }

    Database db(path);
    db.open(Baloo::Database::CreateDatabase);

    FileIndexerConfig config;
    QMimeDatabase mimeDb;

    Transaction tr(db, Transaction::ReadWrite);

    auto shouldDelete = [&tr, &config, &mimeDb](quint64 id) {
        if (!id) {
            return false;
        }

        QString url = tr.documentUrl(id);

        if (!QFile::exists(url)) {
            qDebug() << "not exists: " << url;
            return true;
        }

        if (!config.shouldBeIndexed(url)) {
            qDebug() << "should not be indexed: " << url;
            return true;
        }

        // FIXME: This mimetype is not completely accurate!
        QString mimetype = mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchExtension).name();
        if (!config.shouldMimeTypeBeIndexed(mimetype)) {
            qDebug() << "mimetype should not be indexed: " << url << mimetype;
            return true;
        }

        return false;
    };


    for (const QString& folder : config.includeFolders()) {
        quint64 id = filePathToId(QFile::encodeName(folder));
        qDebug() << "Checking " << folder << id;
        tr.removeRecursively(id, shouldDelete);
    }
    tr.commit();

    QCoreApplication::instance()->quit();
}

