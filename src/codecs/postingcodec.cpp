/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingcodec.h"
#include "enginedebug.h"

using namespace Baloo;

QByteArray PostingCodec::encode(const QVector<quint64>& list)
{
    uint size = list.size() * sizeof(quint64);
    const char* ptr = reinterpret_cast<const char*>(list.constData());

    return QByteArray(ptr, size);
}

QVector<quint64> PostingCodec::decode(const QByteArray& arr)
{
    // A length that is not a multiple of the entry size means the value is
    // truncated or corrupted; none of it is decoded.
    if (arr.size() % qsizetype(sizeof(quint64)) != 0) {
        qCWarning(ENGINE) << "PostingCodec::decode: value length" << arr.size() << "is not a multiple of" << sizeof(quint64) << "- not decoding it";
        return {};
    }

    QVector<quint64> vec;
    vec.resize(arr.size() / qsizetype(sizeof(quint64)));
    memcpy(vec.data(), arr.constData(), arr.size());
    return vec;
}
