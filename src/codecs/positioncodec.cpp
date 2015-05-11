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

#include "positioncodec.h"
#include "positioninfo.h"
#include "coding.h"

using namespace Baloo;

PositionCodec::PositionCodec()
{

}

QByteArray PositionCodec::encode(const QVector<PositionInfo>& list)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (const PositionInfo& pos : list) {
        putFixed64(&data, pos.docId);
        putVarint32(&data, pos.positions.size());
        for (uint p : pos.positions) {
            putVarint32(&data, p);
        }
   }

    return data;
}

QVector<PositionInfo> PositionCodec::decode(const QByteArray& arr)
{
    char* data = const_cast<char*>(arr.data());
    char* end = data + arr.size();

    QVector<PositionInfo> vec;
    while (data < end) {
        PositionInfo info;

        info.docId = *reinterpret_cast<quint64*>(data);
        data += sizeof(quint64);

        quint32 size;
        data = getVarint32Ptr(data, end, &size);

        info.positions.resize(size);
        for (int i = 0; i < size; i++) {
            data = getVarint32Ptr(data, end, &info.positions[i]);
        }

        vec << info;
    }

    return vec;
}
