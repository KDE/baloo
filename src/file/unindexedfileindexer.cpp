/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "unindexedfileindexer.h"

#include "unindexedfileiterator.h"
#include "transaction.h"
#include "fileindexerconfig.h"
#include "basicindexingjob.h"

#include <QMimeDatabase>

using namespace Baloo;

UnindexedFileIndexer::UnindexedFileIndexer(Database* db, const FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
{
}

void UnindexedFileIndexer::run()
{
    QMimeDatabase m_mimeDb;
    QStringList includeFolders = m_config->includeFolders();

    for (const QString& includeFolder : includeFolders) {
        Transaction tr(m_db, Transaction::ReadWrite);
        UnIndexedFileIterator it(m_config, &tr, includeFolder);

        while (!it.next().isEmpty()) {
            BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
                : BasicIndexingJob::MarkForContentIndexing;
            BasicIndexingJob job(it.filePath(), it.mimetype(), level);
            job.index();

            // We handle modified files by simply updating the mTime and filename in the Db and marking them for ContentIndexing
            const quint64 id = job.document().id();
            if (tr.hasDocument(id)) {

                DocumentOperations ops = DocumentTime;
                if (it.cTimeChanged()) {
                    ops |= XAttrTerms;
                    if (tr.documentUrl(id) != it.filePath()) {
                        ops |= (FileNameTerms | DocumentUrl);
                    }
                }
                tr.replaceDocument(job.document(), ops);

                if (it.mTimeChanged()) {
                    tr.setPhaseOne(id);
                }

            } else { // New file
                tr.addDocument(job.document());
            }
        }
        tr.commit();
    }

    Q_EMIT done();
}
