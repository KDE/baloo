/*
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QFile>

#include <KLocalizedString>
#include <QFileInfo>

#include "global.h"
#include "indexer.h"

#include "indexentry.h"

using namespace Baloo;

//  IndexEntry Constructors
//  Wraps Transaction, presuming ReadWrite access

IndexEntry::IndexEntry(const Database &db, QTextStream &out)
    : Transaction(db, TransactionType::ReadWrite)
    , m_out(out) {};

IndexEntry::IndexEntry(Database *db, QTextStream &out)
    : IndexEntry(*db, out) {};

//  Index the given file.
//  Done in the foreground, the user will have to wait until any content
//  indexing done (which for a big file can take a while)

bool IndexEntry::indexFileNow(const QString &fileName)
{
    const QFileInfo fileInfo = QFileInfo(fileName);
    if (!fileInfo.exists()) {
        m_out << "Could not stat file: " << fileName << '\n';
        return false;
    }

    const QString canonicalPath = fileInfo.canonicalFilePath();
    quint64 id = filePathToId(QFile::encodeName(canonicalPath));
    if (id == 0) {
        m_out << "Skipping: " << canonicalPath << " Reason: Bad Document Id for file\n";
        return false;
    }

    if (this->inPhaseOne(id)) {
        m_out << "Skipping: " << canonicalPath << " Reason: Already scheduled for indexing\n";
        return false;
    }

    if (!this->documentData(id).isEmpty()) {
        m_out << "Skipping: " << canonicalPath << " Reason: Already indexed\n";
        return false;
    }

    Indexer indexer(canonicalPath, this);
    m_out << "Indexing " << canonicalPath << '\n';
    indexer.index();
    return true;
}
