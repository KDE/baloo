/*
 * This file is part of the KDE Baloo Project
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
