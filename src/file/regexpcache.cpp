/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2014 Vishesh Handa <me@vhanda.in>

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
    for (const QRegularExpression& filter : qAsConst(m_regexpCache)) {
        if (filter.match(s).hasMatch()) {
            return true;
        }
    }
    return false;
}

void RegExpCache::rebuildCacheFromFilterList(const QStringList& filters)
{
    m_regexpCache.clear();
    for (const QString& filter : filters) {
        QString f = filter;
        f.replace(QLatin1Char('.'), QStringLiteral("\\."));
        f.replace(QLatin1Char('?'), QLatin1Char('.'));
        f.replace(QStringLiteral("*"), QStringLiteral(".*"));
        f = QLatin1String("^") + f + QLatin1String("$");

        m_regexpCache.append(QRegularExpression(f));
    }
}
