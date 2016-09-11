/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BALOO_ID_UTILS_
#define BALOO_ID_UTILS_

#include <qplatformdefs.h>
#include <qglobal.h>

#ifdef Q_OS_WIN
# include <QFileInfo>
#endif

namespace Baloo {

inline quint64 devIdAndInodeToId(quint32 devId, quint32 inode)
{
    quint32 arr[2];
    arr[0] = devId;
    arr[1] = inode;

    return *(reinterpret_cast<quint64*>(arr));
}

/**
 * Convert the QT_STATBUF into a 64 bit unique identifier for the file.
 * This identifier is combination of the device id and inode number.
 */
inline quint64 statBufToId(const QT_STATBUF& stBuf)
{
    // We're loosing 32 bits of info, so this could potentially break
    // on file systems with really large inode and device ids
    return devIdAndInodeToId(static_cast<quint32>(stBuf.st_dev),
                             static_cast<quint32>(stBuf.st_ino));
}

inline quint64 filePathToId(const QByteArray& filePath)
{
    QT_STATBUF statBuf;
#ifndef Q_OS_WIN
    if (QT_LSTAT(filePath.constData(), &statBuf) != 0) {
        return 0;
    }
#else
    if (QT_STAT(filePath.constData(), &statBuf) != 0) {
        return 0;
    }
    if (QFileInfo(filePath).isSymLink()) {
        if (QT_STAT(QFileInfo(filePath).symLinkTarget().toUtf8().constData(), &statBuf) != 0) {
            return 0;
        }
    }
#endif
    return statBufToId(statBuf);
}

inline quint32 idToInode(quint64 id)
{
    quint32* arr = reinterpret_cast<quint32*>(&id);
    return arr[1];
}

inline quint32 idToDeviceId(quint64 id)
{
    quint32* arr = reinterpret_cast<quint32*>(&id);
    return arr[0];
}


template<typename T, typename V>
inline void sortedIdInsert(T& vec, const V& id)
{
    /**
     * search with normal <
     */
    const auto i(std::lower_bound(vec.begin(), vec.end(), id));

    /**
     * end reached or element found smaller?
     * => insert new element!
     */
    if (i == vec.end() || (id != *i))
        vec.insert(i, id);
}

template<typename T, typename V>
inline void sortedIdRemove(T& vec, const V& id)
{
    const int idx = vec.indexOf(id);
    if (idx >= 0) {
        vec.remove(idx);
    }
}


}

#endif
