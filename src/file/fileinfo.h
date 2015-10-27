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

#ifndef BALOO_FILEINFO_H
#define BALOO_FILEINFO_H

#include <QByteArray>
#include <qplatformdefs.h>

namespace Baloo {

class FileInfo
{
public:
    explicit FileInfo(const QByteArray& filePath);

    QByteArray filePath() const { return m_filePath; }
    QByteArray fileName() const { return m_filePath.mid(m_filePath.lastIndexOf('/') + 1); }

    quint32 mtime() const { return m_statBuf.st_mtime; }
    quint32 ctime() const { return m_statBuf.st_ctime; }

    bool isDir() const { return S_ISDIR(m_statBuf.st_mode); }
    bool isFile() const { return S_ISREG(m_statBuf.st_mode); }
    bool exists() const { return m_exists; }
    bool isHidden() const;

private:
    QT_STATBUF m_statBuf;
    QByteArray m_filePath;
    bool m_exists;
};

}

#endif // BALOO_FILEINFO_H
