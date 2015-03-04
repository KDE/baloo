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

#ifndef _BALOO_ID_UTILS_
#define _BALOO_ID_UTILS_

#include <qplatformdefs.h>

namespace Baloo {

/**
 * Convert the QT_STATBUF into a 64 bit unique identifier for the file.
 * This identifier is combination of the device id and inode number.
 */
inline uint64_t statBufToId(const QT_STATBUF& stBuf)
{
    uint32_t arr[2];
    arr[0] = stBuf.st_dev;
    arr[1] = stBuf.st_ino;

    return *(reinterpret_cast<uint64_t*>(arr));
}

inline uint32_t idToInode(uint64_t id)
{
    uint32_t* arr = reinterpret_cast<uint32_t*>(&id);
    return arr[1];
}

inline uint32_t idToDeviceId(uint64_t id)
{
    uint32_t* arr = reinterpret_cast<uint32_t*>(&id);
    return arr[0];
}

}

#endif
