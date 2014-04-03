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

#include "utils.h"
#include "pass_dateperiods.h"

#include "term.h"

#include <klocalizedstring.h>

bool localeWordsSeparatedBySpaces()
{
    return i18nc("Are words of your language separated by spaces (Y/N) ?", "Y") == QLatin1String("Y");
}

int termStart(const Baloo::Term &term)
{
    return term.userData(QLatin1String("start_position")).toInt();
}

int termEnd(const Baloo::Term &term)
{
    return term.userData(QLatin1String("end_position")).toInt();
}

void setTermRange(Baloo::Term &term, int start, int end)
{
    term.setUserData(QLatin1String("start_position"), start);
    term.setUserData(QLatin1String("end_position"), end);
}

QString stringValueIfLiteral(const Baloo::Term &term)
{
    if (!term.property().isNull()) {
        return QString();
    }

    if (term.value().type() != QVariant::String) {
        return QString();
    }

    return term.value().toString();
}

long long int longValueIfLiteral(const Baloo::Term& term, bool *ok)
{
    if (!term.property().isNull()) {
        *ok = false;
        return 0;
    }

    if (term.value().type() != QVariant::LongLong) {
        *ok = false;
        return 0;
    }

    *ok = true;
    return term.value().toLongLong();
}

void copyTermRange(Baloo::Term &target, const Baloo::Term &source)
{
    setTermRange(
        target,
        termStart(source),
        termEnd(source)
    );
}

Baloo::Term fuseTerms(const QList<Baloo::Term> &terms, int first_term_index, int &end_term_index)
{
    Baloo::Term fused_term;
    bool words_separated_by_spaces = localeWordsSeparatedBySpaces();
    bool build_and = true;
    bool build_not = false;
    bool first = true;

    QString and_string = i18n("and");
    QString or_string = i18n("or");
    QString not_string = i18n("not");

    // Fuse terms in nested AND and OR terms. "a AND b OR c" is fused as
    // "(a AND b) OR c"
    for (end_term_index=first_term_index; end_term_index<terms.size(); ++end_term_index) {
        Baloo::Term term = terms.at(end_term_index);
        QVariant value = term.value();

        if (value.type() == QVariant::String) {
            QString content = value.toString().toLower();

            if (content == or_string) {
                // Consume the OR term, the next term will be ORed with the previous
                build_and = false;
                continue;
            } else if (content == and_string ||
                       content == QLatin1String("+")) {
                // Consume the AND term
                build_and = true;
                continue;
            } else if (content == QLatin1String("!") ||
                       content == not_string ||
                       content == QLatin1String("-")) {
                // Consume the negation
                build_not = true;
                continue;
            } else if (content == QLatin1String("(")) {
                // Fuse the nested query
                term = fuseTerms(terms, end_term_index + 1, end_term_index);
            } else if (content == QLatin1String(")")) {
                // Done
                return fused_term;
            } else if (content.size() <= 2 && words_separated_by_spaces) {
                // Ignore small terms, they are typically "to", "a", etc.
                continue;
            }
        }

        // Negate the term if needed
        if (build_not) {
            term.setNegation(!term.isNegated());      // TODO: Replace with a method that properly inverts all the Term's operators and apply De Morgan laws to boolean terms
        }

        // Add term to the fused term
        int start_position = termStart(term);
        int end_position = termEnd(term);

        if (first) {
            fused_term = term;
            first = false;
        } else {
            start_position = qMin(start_position, termStart(fused_term));
            end_position = qMax(end_position, termEnd(fused_term));

            if (build_and) {
                if (fused_term.operation() == Baloo::Term::And) {
                    fused_term.addSubTerm(term);
                } else {
                    fused_term = (fused_term && term);
                }
            } else {
                if (fused_term.operation() == Baloo::Term::Or) {
                    fused_term.addSubTerm(term);
                } else {
                    fused_term = (fused_term || term);
                }
            }
        }

        setTermRange(fused_term, start_position, end_position);

        // Default to AND, and don't invert terms
        build_and = true;
        build_not = false;
    }

    return fused_term;
}
