/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCTERMSCODEC_H
#define BALOO_DOCTERMSCODEC_H

#include <QByteArray>
#include <QVector>

namespace Baloo
{
class DocTermsCodec
{
public:
    DocTermsCodec();

    QByteArray encode(const QVector<QByteArray>& terms);
    QVector<QByteArray> decode(const QByteArray& arr);
};
}

#endif // BALOO_DOCTERMSCODEC_H
