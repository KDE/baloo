/*
   This file is part of the KDE Baloo project.
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

#include "postingcodec.h"

using namespace Baloo;

PostingCodec::PostingCodec()
{
}

QByteArray PostingCodec::encode(const QVector<quint64>& list)
{
    uint size = list.size() * sizeof(quint64);
    const char* ptr = reinterpret_cast<const char*>(list.constData());

    return QByteArray(ptr, size);
}

QVector<quint64> PostingCodec::decode(const QByteArray& arr)
{
    QVector<quint64> vec;
    vec.resize(arr.size() / sizeof(quint64));

    memcpy(vec.data(), arr.constData(), arr.size());
    return vec;
}
