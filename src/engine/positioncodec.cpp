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

#include <QDataStream>

using namespace Baloo;

PositionCodec::PositionCodec()
{

}

QByteArray PositionCodec::encode(const QVector<PositionInfo>& list)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (const PositionInfo& pos : list) {
        stream << pos.docId;
        stream << pos.positions;
    }

    return data;
}

QVector<PositionInfo> PositionCodec::decode(const QByteArray& arr)
{
    QDataStream stream(const_cast<QByteArray*>(&arr), QIODevice::ReadOnly);

    QVector<PositionInfo> vec;
    while (!stream.atEnd()) {
        PositionInfo pos;
        stream >> pos.docId;
        stream >> pos.positions;

        vec << pos;
    }

    return vec;
}
