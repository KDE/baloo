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

#include <stdint.h>
#include <string.h>
#include <string>

namespace Baloo {

// Standard Put... routines append to a string
void putFixed32(QByteArray* dst, quint32 value);
void putFixed64(QByteArray* dst, quint64 value);
void putVarint32(QByteArray* dst, quint32 value);
void putVarint64(QByteArray* dst, quint64 value);

// Standard Get... routines parse a value from the beginning of a Slice
// and advance the slice past the parsed value.
bool getFixed32(QByteArray* input, quint32* value);
bool getFixed64(QByteArray* input, quint64* value);
bool getVarint32(QByteArray* input, quint32* value);
bool getVarint64(QByteArray* input, quint64* value);

// Pointer-based variants of GetVarint...  These either store a value
// in *v and return a pointer just past the parsed value, or return
// NULL on error.  These routines only look at bytes in the range
// [p..limit-1]
extern const char* GetVarint32Ptr(const char* p,const char* limit, quint32* v);
extern const char* getVarint64Ptr(const char* p,const char* limit, quint64* v);

// Returns the length of the varint32 or varint64 encoding of "v"
extern int varintLength(uint64_t v);

// Lower-level versions of Put... that write directly into a character buffer
// REQUIRES: dst has enough space for the value being written
extern void encodeFixed32(char* dst, uint32_t value);
extern void encodeFixed64(char* dst, uint64_t value);

// Lower-level versions of Put... that write directly into a character buffer
// and return a pointer just past the last byte written.
// REQUIRES: dst has enough space for the value being written
extern char* encodeVarint32(char* dst, uint32_t value);
extern char* encodeVarint64(char* dst, uint64_t value);

// Lower-level versions of Get... that read directly from a character buffer
// without any bounds checking.

inline uint32_t decodeFixed32(const char* ptr) {
    if (1 /*port::kLittleEndian*/) {
        // Load the raw bytes
        uint32_t result;
        memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
        return result;
    } else {
        return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
                | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
                | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
                | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
    }
}

inline uint64_t decodeFixed64(const char* ptr) {
    if (1 /*port::kLittleEndian*/) {
        // Load the raw bytes
        uint64_t result;
        memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
        return result;
    } else {
        uint64_t lo = decodeFixed32(ptr);
        uint64_t hi = decodeFixed32(ptr + 4);
        return (hi << 32) | lo;
    }
}

// Internal routine for use by fallback path of GetVarint32Ptr
extern char* getVarint32PtrFallback(char* p, char* limit, uint32_t* value);
inline char* getVarint32Ptr(char* p, char* limit, uint32_t* value)
{
    if (p < limit) {
        uint32_t result = *(reinterpret_cast<const unsigned char*>(p));
        if ((result & 128) == 0) {
            *value = result;
            return p + 1;
        }
    }
    return getVarint32PtrFallback(p, limit, value);
}

}

#endif
