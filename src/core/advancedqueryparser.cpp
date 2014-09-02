/*
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "advancedqueryparser.h"

#include <QRegularExpression>

using namespace Baloo;

AdvancedQueryParser::AdvancedQueryParser()
{
}

Term AdvancedQueryParser::parse(const QString& text)
{
    // Handle brackets
    // Split on AND OR
    // Split key:"?value"? pairs
    QList<Term> subTerms;

    QRegularExpression regexp("(\\w+):\"?(\\w+)\"?");
    auto it = regexp.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString prop = match.captured(1);
        QString value = match.captured(2);

        subTerms << Term(prop, value);
    }
    Term propertyTerm(Term::And, subTerms);
    if (propertyTerm.subTerms().size() == 1) {
        propertyTerm = propertyTerm.subTerm();
    }

    QString remainingText(text);
    remainingText.replace(regexp, QString());
    remainingText = remainingText.simplified();

    if (!remainingText.isEmpty()) {
        Baloo::Term textTerm(QString(), remainingText);
        if (propertyTerm.isEmpty()) {
            return textTerm;
        } else {
            return textTerm && propertyTerm;
        }
    } else {
        return propertyTerm;
    }
}

