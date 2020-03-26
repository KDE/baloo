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

#include "newfileindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>
#include <QFileInfo>

using namespace Baloo;

NewFileIndexer::NewFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& newFiles)
    : m_db(db)
    , m_config(config)
    , m_files(newFiles)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_files.isEmpty());
}

void NewFileIndexer::run()
{
    QMimeDatabase mimeDb;
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    Transaction tr(m_db, Transaction::ReadWrite);

    for (const QString& filePath : qAsConst(m_files)) {
        Q_ASSERT(!filePath.endsWith(QLatin1Char('/')));

        QString mimetype;
        QFileInfo fileInfo(filePath);

        if (fileInfo.isSymLink()) {
            continue;
        }

        if (fileInfo.isDir()) {
            if (!m_config->shouldFolderBeIndexed(filePath)) {
                continue;
            }
            mimetype = QStringLiteral("inode/directory");

        } else {
            QString fileName = filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1);
            if (!m_config->shouldFileBeIndexed(fileName)) {
                continue;
            }
            mimetype = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        }

        BasicIndexingJob job(filePath, mimetype, level);
        if (!job.index()) {
            continue;
        }

        // The same file can be sent twice though it shouldn't be.
        // Lets just silently ignore it instead of crashing
        if (tr.hasDocument(job.document().id())) {
            continue;
        }
        tr.addDocument(job.document());
    }

    tr.commit();
    Q_EMIT done();
}
