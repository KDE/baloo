/* This file is part of the Baloo query parser
   Copyright (c) 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

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

#include "pass_properties.h"
#include "utils.h"

#include "../term.h"

PassProperties::PassProperties()
{
}

void PassProperties::setProperty(const QString &property, Types range)
{
    this->property = property;
    this->range = range;
}

QVariant PassProperties::convertToRange(const QVariant &value) const
{
    switch (range) {
    case Integer:
        if (value.type() == QVariant::LongLong) {
            return value;
        }
        break;

    case IntegerOrDouble:
        if (value.type() == QVariant::LongLong || value.type() == QVariant::Double) {
            return value;
        }
        break;

    case String:
        if (value.type() == QVariant::String) {
            return value;
        }
        break;

    case DateTime:
        if (value.type() == QVariant::DateTime) {
            return value;
        }
        break;

    case Tag:
    case Contact:
    case EmailAddress:
        if (value.type() == QVariant::String) {
            return value;
        }
        break;
    }

    return QVariant();
}

QList<Baloo::Term> PassProperties::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    Baloo::Term term = match.at(0);
    QVariant value = convertToRange(term.value());

    // If the term could be converted to the range of the desired property,
    // then build a comparison
    if (value.isValid()) {
        term.setValue(value);
        term.setProperty(property);

        // Mutate "equals" to "contains" for String ranges
        if (term.comparator() == Baloo::Term::Equal && range == String) {
            term.setComparator(Baloo::Term::Contains);
        }

        rs.append(term);
    }

    return rs;
}
