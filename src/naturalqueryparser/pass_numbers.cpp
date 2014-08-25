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

#include "pass_numbers.h"
#include "utils.h"

#include "term.h"

#include <klocalizedstring.h>

PassNumbers::PassNumbers()
{
    registerNames(0, i18nc("Space-separated list of words meaning 0", "zero nought null"));
    registerNames(1, i18nc("Space-separated list of words meaning 1", "one a first"));
    registerNames(2, i18nc("Space-separated list of words meaning 2", "two second"));
    registerNames(3, i18nc("Space-separated list of words meaning 3", "three third"));
    registerNames(4, i18nc("Space-separated list of words meaning 4", "four fourth"));
    registerNames(5, i18nc("Space-separated list of words meaning 5", "five fifth"));
    registerNames(6, i18nc("Space-separated list of words meaning 6", "six sixth"));
    registerNames(7, i18nc("Space-separated list of words meaning 7", "seven seventh"));
    registerNames(8, i18nc("Space-separated list of words meaning 8", "eight eighth"));
    registerNames(9, i18nc("Space-separated list of words meaning 9", "nine ninth"));
    registerNames(10, i18nc("Space-separated list of words meaning 10", "ten tenth"));
}

void PassNumbers::registerNames(long long int number, const QString &names)
{
    Q_FOREACH(const QString &name, names.split(QLatin1Char(' '))) {
        number_names.insert(name, number);
    }
}

QList<Baloo::Term> PassNumbers::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;

    // Convert a string to a number
    const QString value = stringValueIfLiteral(match.at(0));

    if (number_names.contains(value)) {
        // Named number
        rs.append(Baloo::Term(
            QString(),
            number_names.value(value),
            Baloo::Term::Equal
        ));
    } else {
        // Integer
        bool ok;
        long long int as_integer = value.toLongLong(&ok);

        if (ok) {
            rs.append(Baloo::Term(
                QString(),
                as_integer,
                Baloo::Term::Equal
            ));
        }
    }

    return rs;
}
