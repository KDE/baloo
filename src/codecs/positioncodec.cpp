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
    QByteArray temporaryStorage;

    for (const PositionInfo& pos : list) {
        putFixed64(&data, pos.docId);
        putDifferentialVarInt32(temporaryStorage, &data, pos.positions);
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

        info.docId = decodeFixed64(data);
        data += sizeof(quint64);
        data = getDifferentialVarInt32(data, end, &info.positions);

        vec << info;
    }

    return vec;
}
