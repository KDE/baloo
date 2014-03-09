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

#include "pass_periodnames.h"
#include "pass_dateperiods.h"
#include "utils.h"

#include "../term.h"

#include <klocalizedstring.h>

PassPeriodNames::PassPeriodNames()
{
    registerNames(day_names, i18nc(
        "Day names, starting at the first day of the week (Monday for the Gregorian Calendar)",
        "monday tuesday wednesday thursday friday saturday sunday"
    ));
    registerNames(month_names, i18nc(
        "Month names, starting at the first of the year",
        "january february march april may june july augustus september october november december"
    ));
}

void PassPeriodNames::registerNames(QHash<QString, long long> &table, const QString &names)
{
    QStringList list = names.split(QLatin1Char(' '));

    for (long long i=0; i<list.count(); ++i) {
        table.insert(list.at(i), i + 1);    // Count from 1 as calendars do this
    }
}

QList<Baloo::Term> PassPeriodNames::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    QString name = stringValueIfLiteral(match.at(0)).toLower();

    PassDatePeriods::Period period = PassDatePeriods::VariablePeriod;
    long long value;

    if (day_names.contains(name)) {
        period = PassDatePeriods::DayOfWeek;
        value = day_names.value(name);
    } else if (month_names.contains(name)) {
        period = PassDatePeriods::Month;
        value = month_names.value(name);
    }

    if (period != PassDatePeriods::VariablePeriod) {
        rs.append(Baloo::Term(
            PassDatePeriods::propertyName(period, false),
            value,
            Baloo::Term::Equal
        ));
    }

    return rs;
}
