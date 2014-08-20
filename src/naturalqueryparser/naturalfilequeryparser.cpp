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

#include "naturalfilequeryparser.h"
#include "naturalqueryparser_p.h"
#include "pass_properties.h"
#include "pass_propertyinfo.h"

using namespace Baloo;

struct NaturalFileQueryParser::Private
{
    PassPropertyInfo pass_propertyinfo;
};

NaturalFileQueryParser::NaturalFileQueryParser()
: d(new Private)
{
}

NaturalFileQueryParser::~NaturalFileQueryParser()
{
    delete d;
}

void NaturalFileQueryParser::addSpecificPatterns(int cursor_position, NaturalQueryParser::ParserFlags flags) const
{
    NaturalQueryParser::addSpecificPatterns(cursor_position, flags);

    // Properties associated with any Baloo resource
    passProperties().setProperty(QLatin1String("rating"), PassProperties::Integer);
    runPass(passProperties(), cursor_position,
        i18nc("Numeric rating of a resource", "rated as $1;rated $1;score is $1;score|scored $1;having $1 stars|star"),
        ki18n("Rating (0 to 10)"), CompletionProposal::NoType);
    passProperties().setProperty(QLatin1String("usercomment"), PassProperties::String);
    runPass(passProperties(), cursor_position,
        i18nc("Comment of a resource", "described as $1;description|comment is $1;described|description|comment $1"),
        ki18n("Comment or description"), CompletionProposal::NoType);

    // File-related properties
    passProperties().setProperty(QLatin1String("author"), PassProperties::Contact);
    runPass(passProperties(), cursor_position,
        i18nc("Author of a document", "written|created|composed by $1;author is $1;by $1"),
        ki18n("Author"), CompletionProposal::Contact);
    passProperties().setProperty(QLatin1String("size"), PassProperties::IntegerOrDouble);
    runPass(passProperties(), cursor_position,
        i18nc("Size of a file", "size is $1;size $1;being $1 large;$1 large"),
        ki18n("Size"));
    passProperties().setProperty(QLatin1String("filename"), PassProperties::String);
    runPass(passProperties(), cursor_position,
        i18nc("Name of a file or contact", "name is $1;name $1;named $1"),
        ki18n("Name"));
    passProperties().setProperty(QLatin1String("_k_datecreated"), PassProperties::DateTime);
    runPass(passProperties(), cursor_position,
        i18nc("Date of creation", "created|dated at|on|in|of $1;created|dated $1;creation date|time|datetime is $1"),
        ki18n("Date of creation"), CompletionProposal::DateTime);
    passProperties().setProperty(QLatin1String("_k_datemodified"), PassProperties::DateTime);
    runPass(passProperties(), cursor_position,
        i18nc("Date of last modification", "modified|edited at|on $1;modified|edited $1;modification|edition date|time|datetime is $1"),
        ki18n("Date of last modification"), CompletionProposal::DateTime);

    // Tags
    passProperties().setProperty(QLatin1String("tags"), PassProperties::Tag);
    runPass(passProperties(), cursor_position,
        i18nc("A document is associated with a tag", "tagged as $1;has tag $1;tag is $1;# $1"),
        ki18n("Tag name"), CompletionProposal::Tag);

    // Generic properties using a simpler and more discoverable syntax
    runPass(d->pass_propertyinfo, cursor_position,
        i18nc("$1 is a property name (non-translatable unfortunately) and $2 is the value against which the property is matched. Note that the equal/greater/smaller sign has already been folded in $2", "$1 is $2;$1 $2"),
        ki18n("File property"), CompletionProposal::PropertyName);
}

Query NaturalFileQueryParser::parseQuery(const QString& query, ParserFlags flags)
{
    return NaturalFileQueryParser().parse(query, flags);
}
