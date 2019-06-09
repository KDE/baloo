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

#include "firstrunindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"
#include "filtereddiriterator.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>

using namespace Baloo;

FirstRunIndexer::FirstRunIndexer(Database* db, FileIndexerConfig* config, const QStringList& folders)
    : m_db(db)
    , m_config(config)
    , m_folders(folders)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_folders.isEmpty());
}

void FirstRunIndexer::run()
{
    Q_ASSERT(m_config->isInitialRun());
    {
        Transaction tr(m_db, Transaction::ReadOnly);
        Q_ASSERT_X(tr.size() == 0, "FirstRunIndexer", "The database is not empty on first run");
    }

    QMimeDatabase mimeDb;
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    for (const QString& folder : qAsConst(m_folders)) {
        Transaction tr(m_db, Transaction::ReadWrite);

        FilteredDirIterator it(m_config, folder);
        while (!it.next().isEmpty()) {
            QString mimetype;
            if (it.fileInfo().isDir()) {
                mimetype = QStringLiteral("inode/directory");
            } else {
                mimetype = mimeDb.mimeTypeForFile(it.filePath(), QMimeDatabase::MatchExtension).name();
                if (!m_config->shouldMimeTypeBeIndexed(mimetype)) {
                    continue;
                }
            }

            BasicIndexingJob job(it.filePath(), mimetype, level);
            if (!job.index()) {
                continue;
            }

            // Even though this is the first run, because 2 hard links will resolve to the same id,
            // we land up crashing (due to the asserts in addDocument).
            // Hence we are checking before.
            // FIXME: Silently ignore hard links!
            //
            if (tr.hasDocument(job.document().id())) {
                continue;
            }
            tr.addDocument(job.document());
        }

        // FIXME: This would consume too much memory. We should make some more commits
        //        based on how much memory we consume
        tr.commit();
    }

    m_config->setInitialRun(false);

    Q_EMIT done();
}
