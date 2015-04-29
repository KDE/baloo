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

#ifndef BALOO_POSTINGCODEC_H
#define BALOO_POSTINGCODEC_H

#include "engine_export.h"
#include <QByteArray>
#include <QVector>

namespace Baloo {

class BALOO_ENGINE_EXPORT PostingCodec
{
public:
    PostingCodec();

    QByteArray encode(const QVector<quint64>& list);
    QVector<quint64> decode(const QByteArray& arr);
};

}

#endif // BALOO_POSTINGCODEC_H
