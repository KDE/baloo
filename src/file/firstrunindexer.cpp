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
#include "unindexedfileiterator.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

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

    for (const QString& folder : m_folders) {
        Transaction tr(m_db, Transaction::ReadWrite);

        // FIXME: This checks the mtime of the file as well, and that is not required
        //        during this first run!
        UnIndexedFileIterator it(m_config, &tr, folder);
        while (!it.next().isEmpty()) {
            BasicIndexingJob job(it.filePath(), it.mimetype(), false);
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
