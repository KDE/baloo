/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "xattrindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>

using namespace Baloo;

XAttrIndexer::XAttrIndexer(Database* db, const FileIndexerConfig* config, const QStringList& files)
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
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    Transaction tr(m_db, Transaction::ReadWrite);

    for (const QString& filePath : std::as_const(m_files)) {
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
        BasicIndexingJob job(filePath, mimetype, BasicIndexingJob::NoLevel);
        if (!job.index()) {
            continue;
        }

        // FIXME: This slightly defeats the point of having separate indexers
        //        But we can get xattr changes of a file, even when it doesn't exist
        //        cause we missed its creation somehow
        Baloo::Document doc = job.document();
        if (!tr.hasDocument(doc.id())) {
            doc.setContentIndexing(level == BasicIndexingJob::MarkForContentIndexing);
            tr.addDocument(doc);
            continue;
        }

        tr.replaceDocument(doc, XAttrTerms | DocumentTime | FileNameTerms | DocumentUrl);
    }

    tr.commit();
    Q_EMIT done();
}
