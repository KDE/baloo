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

#ifndef __PASS_TYPEHINTS_H__
#define __PASS_TYPEHINTS_H__

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QUrl>

namespace Baloo { class Term; }

class PassTypeHints
{
    public:
        PassTypeHints();

        QList<Baloo::Term> run(const QList<Baloo::Term> &match) const;

    private:
        void registerHints(const QString &type, const QString &hints);

    private:
        QHash<QString, QString> type_hints;
};

#endif