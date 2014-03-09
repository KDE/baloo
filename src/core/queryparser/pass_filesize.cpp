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

#include "pass_filesize.h"
#include "utils.h"

#include "../term.h"

#include <klocalizedstring.h>

PassFileSize::PassFileSize()
{
    // File size units
    registerUnits(1000LL, i18nc("Lower-case units corresponding to a kilobyte", "kb kilobyte kilobytes"));
    registerUnits(1000000LL, i18nc("Lower-case units corresponding to a megabyte", "mb megabyte megabytes"));
    registerUnits(1000000000LL, i18nc("Lower-case units corresponding to a gigabyte", "gb gigabyte gigabytes"));
    registerUnits(1000000000000LL, i18nc("Lower-case units corresponding to a terabyte", "tb terabyte terabytes"));

    registerUnits(1LL << 10, i18nc("Lower-case units corresponding to a kibibyte", "kib k kibibyte kibibytes"));
    registerUnits(1LL << 20, i18nc("Lower-case units corresponding to a mebibyte", "mib m mebibyte mebibytes"));
    registerUnits(1LL << 30, i18nc("Lower-case units corresponding to a gibibyte", "gib g gibibyte gibibytes"));
    registerUnits(1LL << 40, i18nc("Lower-case units corresponding to a tebibyte", "tib t tebibyte tebibytes"));
}

void PassFileSize::registerUnits(long long int multiplier, const QString &units)
{
    Q_FOREACH(const QString &unit, units.split(QLatin1Char(' '))) {
        multipliers.insert(unit, multiplier);
    }
}

QList<Baloo::Term> PassFileSize::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;

    // Unit
    QString unit = stringValueIfLiteral(match.at(1)).toLower();

    if (multipliers.contains(unit)) {
        long long int multiplier = multipliers.value(unit);
        QVariant value = match.at(0).value();

        if (!match.at(0).property().isNull()) {
            return rs;
        } else if (value.type() == QVariant::Double) {
            value = QVariant(value.toDouble() * double(multiplier));
        } else if (value.type() == QVariant::LongLong) {
            value = QVariant(value.toLongLong() * multiplier);
        } else {
            // String or anything that is not a number
            return rs;
        }

        rs.append(Baloo::Term(
            QLatin1String("size"),
            value,
            Baloo::Term::Equal
        ));
    }

    return rs;
}
