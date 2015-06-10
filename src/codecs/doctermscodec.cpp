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

    QByteArray full;
    full.append(terms.first());
    full.append('\0');

    for (int i = 1; i < terms.size(); i++) {
        const QByteArray term = terms[i];
        const QByteArray prevTerm = terms[i-1];

        if (term.startsWith(prevTerm)) {
            char ch = 1;
            full.append(ch);
            full.append(term.mid(prevTerm.size()));
            full.append('\0');
        } else {
            full.append(term);
            full.append('\0');
        }
    }

    return full;
}

QVector<QByteArray> DocTermsCodec::decode(const QByteArray& full)
{
    Q_ASSERT(full.size());

    QVector<QByteArray> list;

    bool isHalfWord = false;

    int prevWordBoundary = 0;
    for (int i = 0; i < full.size(); i++) {

        if (full[i] == 1) {
            isHalfWord = true;
            prevWordBoundary++;
            continue;
        }

        if (full[i] == '\0') {
            QByteArray arr = QByteArray::fromRawData(full.constData() + prevWordBoundary,
                                                     i - prevWordBoundary);

            if (isHalfWord) {
                list << list.last() + arr;
            } else {
                list << arr;
            }
            prevWordBoundary = i + 1;
            isHalfWord = false;
        }
    }

    return list;
}

