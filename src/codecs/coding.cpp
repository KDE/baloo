/*
 * This file is part of the KDE Baloo project.
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

/*
    Copyright (c) 2011 The LevelDB Authors. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
    * Neither the name of Google Inc. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "coding.h"

namespace Baloo {

static inline int encodeVarint32Internal(char* dst, quint32 v) {
    // Operate on characters as unsigneds
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    static const int B = 128;
    if (v < (1<<7)) {
        ptr[0] = v;
        return 1;
    }
    if (v < (1<<14)) {
        ptr[0] = v | B;
        ptr[1] = v>>7;
        return 2;
    }
    if (v < (1<<21)) {
        ptr[0] = v | B;
        ptr[1] = (v>>7) | B;
        ptr[2] = v>>14;
        return 3;
    }
    if (v < (1<<28)) {
        ptr[0] = v | B;
        ptr[1] = (v>>7) | B;
        ptr[2] = (v>>14) | B;
        ptr[3] = v>>21;
        return 4;
    }

    ptr[0] = v | B;
    ptr[1] = (v>>7) | B;
    ptr[2] = (v>>14) | B;
    ptr[3] = (v>>21) | B;
    ptr[4] = v>>28;
    return 5;
}

static inline void putVarint32Internal(char* dst, quint32 v, int &pos)
{
    pos += encodeVarint32Internal(&dst[pos], v);
}

void putDifferentialVarInt32(QByteArray &temporaryStorage, QByteArray* dst, const QVector<quint32>& values)
{
    temporaryStorage.resize((values.size() + 1) * 5);  // max size, correct size will be held in pos
    int pos = 0;
    putVarint32Internal(temporaryStorage.data(), values.size(), pos);

    quint32 v = 0;
    const auto itEnd = values.cend();
    for (auto it = values.cbegin(); it != itEnd; ++it) {
        const quint32 n = *it;
        putVarint32Internal(temporaryStorage.data(), n - v, pos);
        v = n;
    }
    dst->append(temporaryStorage.constData(), pos);
}

char* getDifferentialVarInt32(char* p, char* limit, QVector<quint32>* values)
{
    quint32 size = 0;
    p = getVarint32Ptr(p, limit, &size);
    values->reserve(size);

    quint32 v = 0;
    while (p && p < limit && size) {
        quint32 n = 0;
        p = getVarint32Ptr(p, limit, &n);

        values->append(n + v);
        v += n;
        size--;
    }

    return p;
}

char* getVarint32PtrFallback(char* p, char* limit, quint32* value)
{
    quint32 result = 0;
    for (quint32 shift = 0; shift <= 28 && p < limit; shift += 7) {
        quint32 byte = *(reinterpret_cast<const unsigned char*>(p));
        p++;
        if (byte & 128) {
            // More bytes are present
            result |= ((byte & 127) << shift);
        } else {
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<char*>(p);
        }
    }
    return nullptr;
}

}
