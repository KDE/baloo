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

#include "xattrindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>

using namespace Baloo;

XAttrIndexer::XAttrIndexer(Database* db, FileIndexerConfig* config, const QStringList& files)
    : m_db(db)
    , m_config(config)
    , m_files(files)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_files.isEmpty());
}

void XAttrIndexer::run()
{
    QMimeDatabase mimeDb;

    Transaction tr(m_db, Transaction::ReadWrite);

    for (const QString& filePath : qAsConst(m_files)) {
        Q_ASSERT(!filePath.endsWith(QLatin1Char('/')));

        QString fileName = filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1);
        if (!m_config->shouldFileBeIndexed(fileName)) {
            continue;
        }

        QString mimetype = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        if (!m_config->shouldMimeTypeBeIndexed(mimetype)) {
            continue;
        }

        // FIXME: The BasicIndexingJob extracts too much info. We only need the xattr
        BasicIndexingJob::IndexingLevel level =
            m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel : BasicIndexingJob::MarkForContentIndexing;
        BasicIndexingJob job(filePath, mimetype, level);
        if (!job.index()) {
            continue;
        }

        // FIXME: This slightly defeats the point of having separate indexers
        //        But we can get xattr changes of a file, even when it doesn't exist
        //        cause we missed its creation somehow
        if (!tr.hasDocument(job.document().id())) {
            tr.addDocument(job.document());
            continue;
        }

        // FIXME: Do we also need to update the ctime of the file?
        tr.replaceDocument(job.document(), XAttrTerms);
    }

    tr.commit();
    Q_EMIT done();
}
