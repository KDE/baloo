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

#include "pass_dateperiods.h"
#include "utils.h"

#include "term.h"

#include <klocalizedstring.h>

PassDatePeriods::PassDatePeriods()
: period(Year),
  value_type(Value),
  value(0)
{
    registerPeriod(Year,
        i18nc("Space-separated list of words representing a year", "year years"));
    registerPeriod(Month,
        i18nc("Space-separated list of words representing a month", "month months"));
    registerPeriod(Week,
        i18nc("Space-separated list of words representing a week", "week weeks"));
    registerPeriod(Day,
        i18nc("Space-separated list of words representing a day", "day days"));
    registerPeriod(Hour,
        i18nc("Space-separated list of words representing an hour", "hour hours"));
    registerPeriod(Minute,
        i18nc("Space-separated list of words representing a minute", "minute minutes"));
    registerPeriod(Second,
        i18nc("Space-separated list of words representing a second", "second seconds"));

    periods.insert(nameOfPeriod(DayOfWeek), DayOfWeek);
}

void PassDatePeriods::registerPeriod(Period period, const QString &names)
{
    Q_FOREACH(const QString &name, names.split(QLatin1Char(' '))) {
        periods.insert(name, period);
    }

    // Also insert the plain English name, used to get the period corresponding
    // to a name extracted from an URL
    periods.insert(nameOfPeriod(period), period);
}

void PassDatePeriods::setKind(PassDatePeriods::Period period, PassDatePeriods::ValueType value_type, int value)
{
    this->period = period;
    this->value_type = value_type;
    this->value = value;
}

QString PassDatePeriods::nameOfPeriod(Period period)
{
    static const char *const period_names[] = {
        "year", "month", "week", "dayofweek", "day", "hour", "minute", "second", ""
    };

    return QLatin1String(period_names[(int)period]);
}

PassDatePeriods::Period PassDatePeriods::periodFromName(const QString &name) const
{
    return periods.value(name);
}

QString PassDatePeriods::propertyName(Period period, bool offset)
{
    return QString::fromLatin1("_k_date_%1_%2")
        .arg(nameOfPeriod(period))
        .arg(QLatin1String(offset ? "offset" : "value"));
}

QList<Baloo::Term> PassDatePeriods::run(const QList<Baloo::Term> &match) const
{
    int value_match_index = 0;

    Period p = period;
    long long v = value;

    if (p == VariablePeriod && !match.isEmpty()) {
        // Parse the period from match.at(0)
        QString period_name = stringValueIfLiteral(match.at(0));

        if (period_name.isNull() || !periods.contains(period_name)) {
            return QList<Baloo::Term>();
        }

        p = periodFromName(period_name);
        value_match_index = 1;
    }

    if (v == 0 && value_match_index < match.count()) {
        // Parse the value either from match.at(0) (there was no period) or
        // match.at(1)
        const Baloo::Term &period_value = match.at(value_match_index);

        if (period_value.value().type() != QVariant::LongLong) {
            return QList<Baloo::Term>();
        }

        v = period_value.value().toLongLong();
    }

    // Create a comparison on the right "property", that will be used in a later
    // pass to build a real date-time object
    return QList<Baloo::Term>() << Baloo::Term(
        propertyName(p, value_type != Value),
        value_type == InvertedOffset ? -v : v,
        Baloo::Term::Equal
    );
}
