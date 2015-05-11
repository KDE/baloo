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

void encodeFixed32(char* buf, uint32_t value) {
    if (1 /*port::kLittleEndian*/) {
        memcpy(buf, &value, sizeof(value));
    } else {
        buf[0] = value & 0xff;
        buf[1] = (value >> 8) & 0xff;
        buf[2] = (value >> 16) & 0xff;
        buf[3] = (value >> 24) & 0xff;
    }
}

void encodeFixed64(char* buf, uint64_t value) {
    if (1 /*port::kLittleEndian*/) {
        memcpy(buf, &value, sizeof(value));
    } else {
        buf[0] = value & 0xff;
        buf[1] = (value >> 8) & 0xff;
        buf[2] = (value >> 16) & 0xff;
        buf[3] = (value >> 24) & 0xff;
        buf[4] = (value >> 32) & 0xff;
        buf[5] = (value >> 40) & 0xff;
        buf[6] = (value >> 48) & 0xff;
        buf[7] = (value >> 56) & 0xff;
    }
}

void putFixed32(QByteArray* dst, quint32 value) {
    char buf[sizeof(value)];
    encodeFixed32(buf, value);
    dst->append(buf, sizeof(buf));
}

void putFixed64(QByteArray* dst, quint64 value) {
    char buf[sizeof(value)];
    encodeFixed64(buf, value);
    dst->append(buf, sizeof(buf));
}

char* encodeVarint32(char* dst, uint32_t v) {
    // Operate on characters as unsigneds
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    static const int B = 128;
    if (v < (1<<7)) {
        *(ptr++) = v;
    } else if (v < (1<<14)) {
        *(ptr++) = v | B;
        *(ptr++) = v>>7;
    } else if (v < (1<<21)) {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = v>>14;
    } else if (v < (1<<28)) {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = (v>>14) | B;
        *(ptr++) = v>>21;
    } else {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = (v>>14) | B;
        *(ptr++) = (v>>21) | B;
        *(ptr++) = v>>28;
    }
    return reinterpret_cast<char*>(ptr);
}

void putVarint32(QByteArray* dst, quint32 v) {
    char buf[5];
    char* ptr = encodeVarint32(buf, v);
    dst->append(buf, ptr - buf);
}

char* encodeVarint64(char* dst, uint64_t v) {
    static const int B = 128;
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    while (v >= B) {
        *(ptr++) = (v & (B-1)) | B;
        v >>= 7;
    }
    *(ptr++) = static_cast<unsigned char>(v);
    return reinterpret_cast<char*>(ptr);
}

void putVarint64(QByteArray* dst, quint64 v) {
    char buf[10];
    char* ptr = encodeVarint64(buf, v);
    dst->append(buf, ptr - buf);
}

int varintLength(uint64_t v) {
    int len = 1;
    while (v >= 128) {
        v >>= 7;
        len++;
    }
    return len;
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
    return NULL;
}

bool getVarint32(QByteArray* input, quint32* value) {
    char* p = input->data();
    char* limit = p + input->size();
    char* q = getVarint32Ptr(p, limit, value);
    if (q == NULL) {
        return false;
    } else {
        *input = QByteArray::fromRawData(q, limit - q);
        return true;
    }
}

const char* getVarint64Ptr(const char* p, const char* limit, quint64* value) {
    quint64 result = 0;
    for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
        quint64 byte = *(reinterpret_cast<const unsigned char*>(p));
        p++;
        if (byte & 128) {
            // More bytes are present
            result |= ((byte & 127) << shift);
        } else {
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<const char*>(p);
        }
    }
    return NULL;
}

bool getVarint64(QByteArray* input, quint64* value) {
    const char* p = input->data();
    const char* limit = p + input->size();
    const char* q = getVarint64Ptr(p, limit, value);
    if (q == NULL) {
        return false;
    } else {
        *input = QByteArray::fromRawData(q, limit - q);
        return true;
    }
}

bool getFixed32(QByteArray* input, quint32* value)
{
    char* p = const_cast<char*>(input->data());
    *value = *reinterpret_cast<quint32*>(p);

    *input = QByteArray::fromRawData(p + sizeof(quint32), input->size() - sizeof(quint32));
    return true;

}

bool getFixed64(QByteArray* input, quint64* value)
{
    char* p = const_cast<char*>(input->data());
    *value = *reinterpret_cast<quint64*>(p);

    *input = QByteArray::fromRawData(p + sizeof(quint64), input->size() - sizeof(quint64));
    return true;
}

}
