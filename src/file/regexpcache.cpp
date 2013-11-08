/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "regexpcache.h"

#include <QStringList>

RegExpCache::RegExpCache()
{
}


RegExpCache::~RegExpCache()
{
}


bool RegExpCache::exactMatch(const QString& s) const
{
    Q_FOREACH(const QRegExp & filter, m_regexpCache) {
        if (filter.exactMatch(s)) {
            return true;
        }
    }
    return false;
}


bool RegExpCache::filenameMatch(const QString& path) const
{
    QString name;
    int i = path.lastIndexOf('/');
    if (i >= 0)
        name = path.mid(i + 1);
    else
        name = path;
    return exactMatch(name);
}


void RegExpCache::rebuildCacheFromFilterList(const QStringList& filters)
{
    m_regexpCache.clear();
    Q_FOREACH(const QString & filter, filters) {
        m_regexpCache.append(QRegExp(filter, Qt::CaseSensitive, QRegExp::Wildcard));
    }
}
