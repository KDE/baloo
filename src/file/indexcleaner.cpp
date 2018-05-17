/*
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "indexcleaner.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include <QDebug>
#include <QFile>
#include <QMimeDatabase>

using namespace Baloo;

IndexCleaner::IndexCleaner(Database* db, FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
{
    Q_ASSERT(db);
    Q_ASSERT(config);
}

void IndexCleaner::run()
{
    QMimeDatabase mimeDb;

    Transaction tr(m_db, Transaction::ReadWrite);

    auto shouldDelete = [&](quint64 id) {
        if (!id) {
            return false;
        }

        QString url = tr.documentUrl(id);

        if (!QFile::exists(url)) {
            qDebug() << "not exists: " << url;
            return true;
        }

        if (!m_config->shouldBeIndexed(url)) {
            qDebug() << "should not be indexed: " << url;
            return true;
        }

        // FIXME: This mimetype is not completely accurate!
        QString mimetype = mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchExtension).name();
        if (!m_config->shouldMimeTypeBeIndexed(mimetype)) {
            qDebug() << "mimetype should not be indexed: " << url << mimetype;
            return true;
        }

        return false;
    };

    const auto includeFolders = m_config->includeFolders();
    for (const QString& folder : includeFolders) {
        quint64 id = filePathToId(QFile::encodeName(folder));
        tr.removeRecursively(id, shouldDelete);
    }
    tr.commit();

    Q_EMIT done();
}
