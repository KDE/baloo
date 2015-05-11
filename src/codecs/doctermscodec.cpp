/*
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

#include "doctermscodec.h"

using namespace Baloo;

DocTermsCodec::DocTermsCodec()
{
}

QByteArray DocTermsCodec::encode(const QVector<QByteArray>& terms)
{
    Q_ASSERT(!terms.isEmpty());

    // We need to put this in one huge byte-array
    // FIXME: Ideally, the data should be provided in such a manner. Not in this vector
    QByteArray full;
    for (const QByteArray& ba : terms) {
        full.append(ba);
        full.append('\0');
    }

    return full;
}

QVector<QByteArray> DocTermsCodec::decode(const QByteArray& full)
{
    Q_ASSERT(full.size());

    QVector<QByteArray> list;

    int prevWordBoundary = 0;
    for (int i = 0; i < full.size(); i++) {
        if (full[i] == '\0') {
            QByteArray arr = QByteArray::fromRawData(full.constData() + prevWordBoundary,
                                                     i - prevWordBoundary);

            list << arr;
            prevWordBoundary = i + 1;
            i++;
        }
    }

    return list;
}

