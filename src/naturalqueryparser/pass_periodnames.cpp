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

#include "term.h"

#include <kcalendarsystem.h>
#include <klocale.h>

PassPeriodNames::PassPeriodNames()
{
    const KCalendarSystem *cal = KLocale::global()->calendar();

    // List of all the day names (try to get as many days as the calendar can provide)
    for (int day=1;
         insertName(day_names,
                    day,
                    cal->weekDayName(day, KCalendarSystem::ShortDayName),
                    cal->weekDayName(day, KCalendarSystem::LongDayName));
         ++day) {
    }

    // List of all the month names (NOTE: Using the current year, and a previous
    // year that is a leap year (or not a leap year if the current year is one))
    int years[2];

    years[0] = cal->year(QDate::currentDate());
    years[1] = years[0];

    while (cal->isLeapYear(years[0]) == cal->isLeapYear(years[1])) {
        --years[1];
    }

    for (int year=0; year<2; ++year) {
        for (int month=1;
             insertName(month_names,
                        month,
                        cal->monthName(month, years[year], KCalendarSystem::ShortName),
                        cal->monthName(month, years[year], KCalendarSystem::LongName));
             ++month) {
        }
    }
}

bool PassPeriodNames::insertName(QHash<QString, int> &hash, int value, const QString &shortName, const QString &longName)
{
    if (shortName.isEmpty() || longName.isEmpty()) {
        return false;
    }

    hash.insert(shortName.toLower(), value);
    hash.insert(longName.toLower(), value);

    return true;
}

QList<Baloo::Term> PassPeriodNames::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    QString name = stringValueIfLiteral(match.at(0)).toLower();

    PassDatePeriods::Period period = PassDatePeriods::VariablePeriod;
    int value;

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
            QVariant((long long)value),
            Baloo::Term::Equal
        ));
    }

    return rs;
}
