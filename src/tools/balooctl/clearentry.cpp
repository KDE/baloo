/*
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QFile>

#include <KLocalizedString>
#include <QFileInfo>
#include <QTextStream>

#include "database.h"
#include "global.h"
#include "transaction.h"

#include "clearentry.h"

using namespace Baloo;

//  ClearEntry Constructors
//  Wraps Transaction, presuming ReadWrite access

ClearEntry::ClearEntry(const Database &db, QTextStream &out)
    : Transaction(db, TransactionType::ReadWrite)
    , m_out(out) {};

ClearEntry::ClearEntry(Database *db, QTextStream &out)
    : ClearEntry(*db, out) {};

//  ClearEntry::clearEntryNow
//  Removes the entry for the file from the index
//  Done in the foreground, the user will have to wait until it's done

bool ClearEntry::clearEntryNow(const QString &fileName)
{
    const auto fileInfo = QFileInfo(fileName);
    const QString canonicalPath = fileInfo.canonicalFilePath();
    quint64 id = filePathToId(QFile::encodeName(canonicalPath));

    if (id) {
        m_out << "Clearing " << canonicalPath << '\n';
    } else {
        id = this->documentId(QFile::encodeName(fileName));
        if (id == 0) {
            m_out << "File not found on filesystem or in DB: " << fileName << '\n';
            return false;
        } else {
            m_out << "File has been deleted, clearing from DB: " << fileName << '\n';
        }
    }

    this->removeDocument(id);
    return true;
}
