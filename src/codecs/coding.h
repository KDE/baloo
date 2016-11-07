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

#ifndef BALOO_STORAGE_LEVELDB_UTIL_CODING_H
#define BALOO_STORAGE_LEVELDB_UTIL_CODING_H

#include <QByteArray>
#include <QVector>

namespace Baloo {

/*
 * This is a stripped down version of various encode/decode functions for
 * 32/64 bit fixed/variable data types. If you need other functions than the
 * ones available here you can take a look in the git baloo history
 */

inline void putFixed64(QByteArray* dst, quint64 value)
{
    dst->append(reinterpret_cast<const char*>(&value), sizeof(value));
}

/*
 * temporaryStorage is used to avoid an internal allocation of a temporary
 * buffer which is needed for serialization. Since this function is normally
 * called inside a loop, the temporary buffer must not be reallocated on every
 * call.
 */
void putDifferentialVarInt32(QByteArray &temporaryStorage, QByteArray* dst, const QVector<quint32>& values);
char* getDifferentialVarInt32(char* input, char* limit, QVector<quint32>* values);
extern const char* getVarint32Ptr(const char* p, const char* limit, quint32* v);

inline quint64 decodeFixed64(const char* ptr)
{
    // Load the raw bytes
    quint64 result;
    memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
    return result;
}

// Internal routine for use by fallback path of GetVarint32Ptr
extern char* getVarint32PtrFallback(char* p, char* limit, quint32* value);
inline char* getVarint32Ptr(char* p, char* limit, quint32* value)
{
    if (p < limit) {
        quint32 result = *(reinterpret_cast<const unsigned char*>(p));
        if ((result & 128) == 0) {
            *value = result;
            return p + 1;
        }
    }
    return getVarint32PtrFallback(p, limit, value);
}

}

#endif
