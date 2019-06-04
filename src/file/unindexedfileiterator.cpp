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

#include "unindexedfileiterator.h"
#include "fileindexerconfig.h"
#include "idutils.h"
#include "transaction.h"
#include "baloodebug.h"

#include <QFileInfo>
#include <QDateTime>

using namespace Baloo;

UnIndexedFileIterator::UnIndexedFileIterator(const FileIndexerConfig* config, Transaction* transaction, const QString& folder)
    : m_config(config)
    , m_transaction(transaction)
    , m_iter(config, folder, FilteredDirIterator::FilesAndDirs)
    , m_mTimeChanged(false)
    , m_cTimeChanged(false)
{
}

UnIndexedFileIterator::~UnIndexedFileIterator()
{
}

QString UnIndexedFileIterator::filePath() const
{
    return m_iter.filePath();
}

QString UnIndexedFileIterator::mimetype() const
{
    return m_mimetype;
}

bool UnIndexedFileIterator::mTimeChanged() const
{
    return m_mTimeChanged;
}

bool UnIndexedFileIterator::cTimeChanged() const
{
    return m_cTimeChanged;
}

QString UnIndexedFileIterator::next()
{
    while (1) {
        const QString filePath = m_iter.next();
        m_mTimeChanged = false;
        m_cTimeChanged = false;

        if (filePath.isEmpty()) {
            m_mimetype.clear();
            return QString();
        }

        if (shouldIndex(filePath)) {
            return filePath;
        }
    }
}

bool UnIndexedFileIterator::shouldIndex(const QString& filePath)
{
    const QFileInfo fileInfo = m_iter.fileInfo();
    if (!fileInfo.exists())
        return false;

    quint64 fileId = filePathToId(QFile::encodeName(filePath));
    if (!fileId) {
        // stat has failed, e.g. when file has been deleted after iteration
        return false;
    }

    DocumentTimeDB::TimeInfo timeInfo = m_transaction->documentTimeInfo(fileId);

    // A folders mtime is updated when a new file is added / removed / renamed
    // we don't really need to reindex a folder when that happens
    // In fact, we never need to reindex a folder
    if (timeInfo.mTime && fileInfo.isDir()) {
        return false;
    }

    if (timeInfo.mTime != fileInfo.lastModified().toTime_t()) {
        m_mTimeChanged = true;
    }

    auto fileMTime = fileInfo.metadataChangeTime().toTime_t();
    if (timeInfo.cTime != fileMTime) {
        m_cTimeChanged = true;
    }

    if (m_mTimeChanged || m_cTimeChanged) {
        if (fileInfo.isDir()) {
            m_mimetype = QStringLiteral("inode/directory");
            return true;
        }

        // This mimetype may not be completely accurate, but that's okay. This is
        // just the initial phase of indexing. The second phase can try to find
        // a more accurate mimetype.
        m_mimetype = m_mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        if (!m_config->shouldMimeTypeBeIndexed(m_mimetype)) {
            return false;
        }

        qCDebug(BALOO) << "mtime/ctime changed:"
            << timeInfo.mTime << fileInfo.lastModified().toTime_t()
            << timeInfo.cTime << fileMTime;
        return true;
    }

    return false;
}
