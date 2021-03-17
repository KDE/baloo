/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILEINFO_H
#define BALOO_FILEINFO_H

#include <QByteArray>
#include <qplatformdefs.h>

namespace Baloo
{
class FileInfo
{
public:
    explicit FileInfo(const QByteArray& filePath);

    QByteArray filePath() const
    {
        return m_filePath;
    }
    QByteArray fileName() const
    {
        return m_filePath.mid(m_filePath.lastIndexOf('/') + 1);
    }

    quint32 mtime() const
    {
        return m_statBuf.st_mtime;
    }
    quint32 ctime() const
    {
        return m_statBuf.st_ctime;
    }

    bool isDir() const
    {
        return S_ISDIR(m_statBuf.st_mode);
    }
    bool isFile() const
    {
        return S_ISREG(m_statBuf.st_mode);
    }
    bool exists() const
    {
        return m_exists;
    }
    bool isHidden() const;

private:
    QT_STATBUF m_statBuf;
    QByteArray m_filePath;
    bool m_exists;
};

}

#endif // BALOO_FILEINFO_H
