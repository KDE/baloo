/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEPOMUKTOOLS_H
#define NEPOMUKTOOLS_H

#include <KUrl>
#include <QtCore/QStringList>
#include <Soprano/Node>

namespace Nepomuk2
{
const int MAX_SPLIT_LIST_ITEMS = 20;

/**
 * Convert a list or set or QUrls into a list of N3 formatted strings.
 */
template<typename T> QStringList resourcesToN3(const T& urls)
{
    QStringList n3;
    Q_FOREACH(const QUrl & url, urls) {
        n3 << Soprano::Node::resourceToN3(url);
    }
    return n3;
}

/**
 * Split a list into several lists, each not containing more than \p max items
 */
template<typename T> QList<QList<T> > splitList(const QList<T>& list, int max = MAX_SPLIT_LIST_ITEMS)
{
    QList<QList<T> > splitted;
    int i = 0;
    QList<T> single;
    foreach(const T & item, list) {
        single.append(item);
        if (++i >= max) {
            splitted << single;
            single.clear();
        }
    }
    return splitted;
}
}

#endif // NEPOMUKTOOLS_H
