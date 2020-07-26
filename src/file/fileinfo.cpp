/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fileinfo.h"
#include "idutils.h"

using namespace Baloo;

FileInfo::FileInfo(const QByteArray& filePath)
    : m_filePath(filePath)
    , m_exists(true)
{
    Q_ASSERT(!filePath.endsWith('/'));
    if (filePathToStat(filePath, m_statBuf) != 0) {
        m_exists = false;
        memset(&m_statBuf, 0, sizeof(m_statBuf));
    }
}

bool FileInfo::isHidden() const
{
    int pos = m_filePath.lastIndexOf('/');
    return m_filePath[pos + 1] == '.';
}
