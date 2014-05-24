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

#include "pass_comparators.h"

PassComparators::PassComparators()
: comparator(Baloo::Term::Equal)
{
}

void PassComparators::setComparator(Baloo::Term::Comparator comparator)
{
    this->comparator = comparator;
}

QList<Baloo::Term> PassComparators::run(const QList<Baloo::Term> &match) const
{
    Baloo::Term term(match.at(0));

    // Set the comparator of the term and ignore the property and the value, that
    // will be (or are already) filled by other passes
    term.setComparator(comparator);

    return QList<Baloo::Term>() << term;
}
