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

#include "completionproposal.h"

#include "klocalizedstring.h"

using namespace Baloo;

struct CompletionProposal::Private
{
    QStringList pattern;
    int last_matched_part;
    int position;
    int length;
    CompletionProposal::Type type;
    KLocalizedString description;
};

CompletionProposal::CompletionProposal(const QStringList &pattern,
                                       int last_matched_part,
                                       int position,
                                       int length,
                                       Type type,
                                       const KLocalizedString &description)
: d(new Private)
{
    d->pattern = pattern;
    d->last_matched_part = last_matched_part;
    d->position = position;
    d->length = length;
    d->type = type;
    d->description = description;
}

CompletionProposal::~CompletionProposal()
{
    delete d;
}

int CompletionProposal::position() const
{
    return d->position;
}

int CompletionProposal::length() const
{
    return d->length;
}

int CompletionProposal::lastMatchedPart() const
{
    return d->last_matched_part;
}

QStringList CompletionProposal::pattern() const
{
    return d->pattern;
}

CompletionProposal::Type CompletionProposal::type() const
{
    return d->type;
}

KLocalizedString CompletionProposal::description() const
{
    return d->description;
}
