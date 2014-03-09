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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "../term.h"

#include <QtCore/QString>
#include <QtCore/QList>

bool localeWordsSeparatedBySpaces();

QString stringValueIfLiteral(const Baloo::Term &term);
long long longValueIfLiteral(const Baloo::Term &term, bool *ok);
void copyTermPosition(Baloo::Term &target, const Baloo::Term &source);

Baloo::Term fuseTerms(const QList<Baloo::Term> &terms,
                      int first_term_index,
                      int &end_term_index);

#endif