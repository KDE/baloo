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

#include "modifiedfileindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"
#include "idutils.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

using namespace Baloo;

ModifiedFileIndexer::ModifiedFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& files)
    : m_db(db)
    , m_config(config)
    , m_files(files)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_files.isEmpty());
}

void ModifiedFileIndexer::run()
{
    QMimeDatabase mimeDb;
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    Transaction tr(m_db, Transaction::ReadWrite);

    for (const QString& filePath : qAsConst(m_files)) {
        Q_ASSERT(!filePath.endsWith('/'));

        QString fileName = filePath.mid(filePath.lastIndexOf('/') + 1);
        if (!m_config->shouldFileBeIndexed(fileName)) {
            continue;
        }

        QString mimetype = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        if (!m_config->shouldMimeTypeBeIndexed(mimetype)) {
            continue;
        }

        quint64 fileId = filePathToId(QFile::encodeName(filePath));
        if (!fileId) {
            continue;
        }

        DocumentTimeDB::TimeInfo timeInfo = tr.documentTimeInfo(fileId);

        // FIXME: Using QFileInfo over here is quite expensive!
        QFileInfo fileInfo(filePath);

        // A folders mtime is updated when a new file is added / removed / renamed
        // we don't really need to reindex a folder when that happens
        // In fact, we never need to reindex a folder
        if (timeInfo.mTime && fileInfo.isDir()) {
            continue;
        }

        bool mTimeChanged = timeInfo.mTime != fileInfo.lastModified().toTime_t();
        bool cTimeChanged = timeInfo.cTime != fileInfo.metadataChangeTime().toTime_t();

        if (!mTimeChanged && !cTimeChanged) {
            continue;
        }

        // FIXME: The BasicIndexingJob extracts too much info. We only need the time
        BasicIndexingJob job(filePath, mimetype, level);
        if (!job.index()) {
            continue;
        }

        // we can get modified events for files which do not exist
        // cause Baloo was not running and missed those events
        if (tr.hasDocument(job.document().id())) {
            if (cTimeChanged) {
                tr.replaceDocument(job.document(), XAttrTerms | DocumentTime | FileNameTerms | DocumentUrl);
            } else {
                tr.replaceDocument(job.document(), DocumentTime);
            }
        }
        else {
            tr.addDocument(job.document());
        }
    }

    tr.commit();
    Q_EMIT done();
}
