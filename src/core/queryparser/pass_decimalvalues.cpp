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

#include "pass_decimalvalues.h"
#include "utils.h"

#include "term.h"

static double scales[] = {
    1.0,
    0.1,
    0.01,
    0.001,
    0.0001,
    0.00001,
    0.000001,
    0.0000001,
    0.00000001,
    0.000000001,
    0.0000000001,
    0.00000000001,
    0.000000000001
};

QList<Baloo::Term> PassDecimalValues::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    bool has_integer_part;
    bool has_decimal_part;
    long long integer_part = longValueIfLiteral(match.at(0), &has_integer_part);
    long long decimal_part = longValueIfLiteral(match.at(1), &has_decimal_part);
    int decimal_length = termEnd(match.at(1)) - termStart(match.at(1)) + 1;

    if (has_integer_part &&
        has_decimal_part &&
        decimal_length <= 12) {

        double scale = scales[decimal_length];

        rs.append(Baloo::Term(
            QString(),
            double(integer_part) + (double(decimal_part) * scale),
            Baloo::Term::Equal
        ));
    }

    return rs;
}
