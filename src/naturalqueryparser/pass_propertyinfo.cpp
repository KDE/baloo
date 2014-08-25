/* This file is part of the Baloo query parser
   Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation,
   or any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pass_propertyinfo.h"
#include "utils.h"

#include <KFileMetaData/PropertyInfo>

PassPropertyInfo::PassPropertyInfo()
{
    // Query PropertyInfo for all the valid property names
    int lastProperty = KFileMetaData::Property::LastProperty;

    for (int i=0; i<lastProperty; ++i) {
        KFileMetaData::PropertyInfo info((KFileMetaData::Property::Property)i);

        if (!info.name().isEmpty()) {
            validPropertyNames.insert(info.name());
        }
    }
}

QList<Baloo::Term> PassPropertyInfo::run(const QList<Baloo::Term> &match) const
{
    Q_ASSERT(match.count() == 2);
    QList<Baloo::Term> rs;
    QString propertyName = stringValueIfLiteral(match.at(0));
    Baloo::Term term = match.at(1);

    // Build a simple property comparison if the property name is something valid
    if (!propertyName.isNull() && validPropertyNames.contains(propertyName)) {
        term.setProperty(propertyName);
        rs.append(term);
    }

    return rs;
}
