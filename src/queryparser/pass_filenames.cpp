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

#include "pass_filenames.h"
#include "utils.h"

#include "term.h"

#include <QtCore/QRegExp>

QList<Baloo::Term> PassFileNames::run(const QList<Baloo::Term> &match) const
{
    QList<Baloo::Term> rs;
    const QString value = stringValueIfLiteral(match.at(0));

    if (value.contains(QLatin1Char('.'))) {
        if (value.contains(QLatin1Char('*')) || value.contains(QLatin1Char('?'))) {
            // Filename globbing (original code here from Vishesh Handa)
            QString regex = QRegExp::escape(value);

            regex.replace(QLatin1String("\\*"), QLatin1String(".*"));
            regex.replace(QLatin1String("\\?"), QLatin1String("."));
            regex.replace(QLatin1String("\\"), QLatin1String("\\\\"));
            regex.prepend(QLatin1Char('^'));
            regex.append(QLatin1Char('$'));

            rs.append(Baloo::Term(
                QLatin1String("filename"),
                QRegExp(regex),             // NOTE: QRegExp::toString gives the literal value entered by the user, but some backends may want to use the actual QRegExp object
                Baloo::Term::Contains
            ));
        } else {
            rs.append(Baloo::Term(
                QLatin1String("filename"),
                value,
                Baloo::Term::Contains
            ));
        }
    }

    return rs;
}
