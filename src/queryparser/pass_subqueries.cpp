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

#include "pass_subqueries.h"
#include "utils.h"

#include "term.h"

void PassSubqueries::setProperty(const QString &property)
{
    this->property = property;
}

QList<Baloo::Term> PassSubqueries::run(const QList<Baloo::Term> &match) const
{
    // Fuse the matched terms (... in "related to ... ,") into a subquery
    int end_index;
    Baloo::Term fused_term = fuseTerms(match, 0, end_index);

    fused_term.setProperty(property);               // FIXME: Do backends understand terms having a property and subterms and no value? (for instance "related = AND(term, term, term)")
    fused_term.setComparator(Baloo::Term::Equal);

    return QList<Baloo::Term>() << fused_term;
}
