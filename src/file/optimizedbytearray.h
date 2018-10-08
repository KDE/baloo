/*
 * This file is part of the KDE Nepomuk Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#ifndef OPTIMIZED_BYTE_ARRAY_H
#define OPTIMIZED_BYTE_ARRAY_H

#include <QHash>
#include <QVector>
#include <QByteArray>
#include <QSet>

/**
 * This class holds a QByteArray which corresponds to a file url in a
 * more memory efficient manner.
 *
 * It does so by spliting the url along the directory separator '/', and
 * saving each path separately in a QVector. This is more memory efficient
 * than storing it together cause it uses a QSet<QByteArray> cache to lookup
 * each part of the vector. This way, multiple OptimizedByteArrays share common
 * parts of the url. This happens because Qt's containers are reference counted.
 *
 * \author Vishesh Handa <me@vhanda.in>
 *
 */
class OptimizedByteArray
{
public:
    OptimizedByteArray() {}

    OptimizedByteArray(const QByteArray& array, QSet<QByteArray>& cache) {
        QList<QByteArray> list = array.split('/');
        QVector<QByteArray> vec;
        vec.reserve(list.size());
        Q_FOREACH (const QByteArray& ba, list) {
            if (!ba.isEmpty())
                vec << ba;
        }

        m_data.reserve(vec.size());
        Q_FOREACH (const QByteArray& arr, vec) {
            QSet< QByteArray >::iterator it = cache.find(arr);
            if (it != cache.end())
                m_data.append(*it);
            else
                m_data.append(*cache.insert(arr));
        }
    }

    QByteArray toByteArray() const {
        int size = 0;
        Q_FOREACH (const QByteArray& arr, m_data)
            size += arr.size() + 1;

        QByteArray array;
        array.reserve(size);

        Q_FOREACH (const QByteArray& arr, m_data) {
            array.append('/');
            array.append(arr);
        }

        return array;
    }

    bool operator==(const OptimizedByteArray& other) const {
        return m_data == other.m_data;
    }

private:
    QVector<QByteArray> m_data;
};

uint qHash(const OptimizedByteArray& arr)
{
    return qHash(arr.toByteArray());
}

Q_DECLARE_METATYPE(OptimizedByteArray)
Q_DECLARE_TYPEINFO(OptimizedByteArray, Q_MOVABLE_TYPE);

#endif // OPTIMIZED_BYTE_ARRAY_H
