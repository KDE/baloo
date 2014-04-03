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

#include "pass_datevalues.h"
#include "pass_dateperiods.h"
#include "utils.h"

#include "../term.h"

PassDateValues::PassDateValues()
: pm(false)
{
}

void PassDateValues::setPm(bool pm)
{
    this->pm = pm;
}

QList<Baloo::Term> PassDateValues::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    bool valid_input = true;
    bool progress = false;

    static const PassDatePeriods::Period periods[7] = {
        PassDatePeriods::Year, PassDatePeriods::Month, PassDatePeriods::Day,
        PassDatePeriods::DayOfWeek, PassDatePeriods::Hour, PassDatePeriods::Minute,
        PassDatePeriods::Second
    };
    // Conservative minimum values (not every calendar already reached year 2000+)
    static const int min_values[7] = {
        0 /* Y */, 1 /* M */, 1 /* D */, 1 /* DW */, 0 /* H */, 0 /* M */, 0 /* S */
    };
    // Conservative maximum values (some calendars may have months of 100+ days)
    static const int max_values[7] = {
        1<<30 /* Y */, 60 /* M */, 500 /* D */, 7 /* DW */, 24 /* H */, 60 /* M */, 60 /* S */
    };

    // See if a match sets a value for any period
    for (int i=0; i<7; ++i) {
        PassDatePeriods::Period period = periods[i];

        if (i < match.count() && match.at(i).value().isValid()) {
            const Baloo::Term &term = match.at(i);
            bool value_is_integer;
            long long value = term.value().toLongLong(&value_is_integer);

            if (term.property().startsWith("_k_date")) {
                // The term is already a date part, no need to change it
                rs.append(term);
                continue;
            }

            if (!term.property().isNull()) {
                // This term has already a property (and is therefore a comparison)
                valid_input = false;
                break;
            }

            if (!value_is_integer) {
                // Something that is neither a date part nor a valid integer
                valid_input = false;
                break;
            }

            if (value < min_values[i] || value > max_values[i]) {
                // Integer too big or too small to be a valid date part
                valid_input = false;
                break;
            }

            if (period == PassDatePeriods::Hour && pm) {
                value += 12;
            }

            // Build a comparison of the right type
            Baloo::Term comparison(
                PassDatePeriods::propertyName(period, false),
                value,
                Baloo::Term::Equal
            );

            copyTermRange(comparison, term);

            progress = true;
            rs.append(comparison);
        }
    }

    if (!valid_input || !progress) {
        rs.clear();
    }

    return rs;
}
