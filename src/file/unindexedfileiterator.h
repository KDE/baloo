/*
 * This file is part of the KDE Baloo project.
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

#ifndef BALOO_UNINDEXEDFILEITERATOR_H
#define BALOO_UNINDEXEDFILEITERATOR_H

#include "filtereddiriterator.h"

#include <QMimeDatabase>

namespace Baloo {

class Transaction;

/**
 * Iterate over all the files (and directories) under a specific directory which require
 * indexing. This checks the following -
 * - Config include / exclude path
 * - Config filters
 * - Config mimetype filters
 * - Datbase for mtime differences
 */
class UnIndexedFileIterator
{
public:
    UnIndexedFileIterator(FileIndexerConfig* config, Transaction* transaction, const QString& folder);
    ~UnIndexedFileIterator();

    QString next();
    QString filePath() const;
    QString mimetype() const;
    bool mTimeChanged() const;
    bool cTimeChanged() const;

private:
    bool shouldIndex(const QString& filePath, const QString& mimetype);

    FileIndexerConfig* m_config;
    Transaction* m_transaction;
    FilteredDirIterator m_iter;

    QMimeDatabase m_mimeDb;
    QString m_mimetype;

    bool m_mTimeChanged;
    bool m_cTimeChanged;
};

}

#endif // BALOO_UNINDEXEDFILEITERATOR_H
