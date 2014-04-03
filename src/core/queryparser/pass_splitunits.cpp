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

#include "pass_splitunits.h"
#include "utils.h"

#include "../term.h"

#include <klocalizedstring.h>

PassSplitUnits::PassSplitUnits()
: known_units(
    QSet<QString>::fromList(
        i18nc(
            "List of lowercase prefixes or suffix that need to be split from values",
            "k m g b kb mb gb tb kib mib gib tib h am pm th rd nd st"
        ).split(QLatin1Char(' '))
    )
  )
{
}

QList<Baloo::Term> PassSplitUnits::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    Baloo::Term value_term;
    Baloo::Term unit_term;

    QString value = stringValueIfLiteral(match.at(0));
    int value_position = termStart(match.at(0));

    if (value.isNull()) {
        return rs;
    }

    // Possible prefix
    QString prefix;

    for (int i=0; i<value.size() && value.at(i).isLetter(); ++i) {
        prefix.append(value.at(i).toLower());
    }

    if (prefix.size() < value.size() && known_units.contains(prefix)) {
        unit_term.setValue(prefix);

        setTermRange(unit_term, value_position, value_position + prefix.size() - 1);

        value = value.mid(prefix.size());
        value_position += prefix.size();
    }

    // Possible postfix
    QString postfix;

    for (int i=value.size()-1; i>=0 && value.at(i).isLetter(); --i) {
        postfix.prepend(value.at(i).toLower());
    }

    if (postfix.size() < value.size() && known_units.contains(postfix)) {
        value.resize(value.size() - postfix.size());

        unit_term.setValue(postfix);

        setTermRange(
            unit_term,
            value_position + value.size(),
            value_position + value.size() + postfix.size() - 1
        );
    }

    // Value
    value_term.setValue(value);

    setTermRange(value_term, value_position, value_position + value.size() - 1);

    if (unit_term.value().isValid()) {
        rs.append(value_term);
        rs.append(unit_term);
    }

    return rs;
}
