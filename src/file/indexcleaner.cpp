/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "indexcleaner.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include "baloodebug.h"

#include <QFile>

using namespace Baloo;

IndexCleaner::IndexCleaner(Database *db, FileIndexerConfig *config, HiddenFiles hiddenFiles)
    : m_db(db)
    , m_config(config)
    , m_hiddenFiles(hiddenFiles)
{
    Q_ASSERT(db);
    Q_ASSERT(config);
}

void IndexCleaner::run()
{
    auto shouldDelete = [&](const QByteArray &baUrl) {
        const QString url = QFile::decodeName(baUrl);

        if (!QFile::exists(url)) {
            qCDebug(BALOO) << "not exists: " << url;
            return true;
        }

        if (!m_config->shouldBeIndexed(url)) {
            qCDebug(BALOO) << "should not be indexed: " << url;
            return true;
        }

        return false;
    };

    const auto excludeFolders = m_config->excludeFolders();
    for (const QString& folder : excludeFolders) {
        Transaction tr(m_db, Transaction::ReadWrite);

        quint64 id = filePathToId(QFile::encodeName(folder));
        if (id > 0 && tr.hasDocument(id)) {
            tr.removeRecursively(id, shouldDelete);
        }
        tr.commit();
    }

    if (m_hiddenFiles == HiddenFiles::RemoveFromIndex) {
        // Hidden files and folders indexed while the setting was on sit below folders that stay
        // in the index, so the walk of the excluded folders above never reaches them. Go through
        // the included folders as well, looking at every document on the way down.
        const auto includeFolders = m_config->includeFolders();
        for (const QString &folder : includeFolders) {
            Transaction tr(m_db, Transaction::ReadWrite);

            quint64 id = filePathToId(QFile::encodeName(folder));
            if (id > 0 && tr.hasDocument(id)) {
                tr.removeRecursively(id, shouldDelete, WriteTransaction::RemovalWalk::VisitEveryDocument);
            }
            tr.commit();
        }
    }

    Q_EMIT done();
}

#include "moc_indexcleaner.cpp"
