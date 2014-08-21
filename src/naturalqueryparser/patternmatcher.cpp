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

#include "patternmatcher.h"
#include "naturalqueryparser.h"
#include "utils.h"

#include <QRegExp>

PatternMatcher::PatternMatcher(const Baloo::NaturalQueryParser *parser,
                               QList<Baloo::Term> &terms,
                               int cursor_position,
                               const QStringList &pattern,
                               Baloo::CompletionProposal::Type completion_type,
                               const KLocalizedString &completion_description)
: parser(parser),
  terms(terms),
  cursor_position(cursor_position),
  pattern(pattern),
  completion_type(completion_type),
  completion_description(completion_description),
  capture_count(captureCount())
{
}

int PatternMatcher::captureCount() const
{
    int max_capture = 0;
    int capture;

    Q_FOREACH(const QString &p, pattern) {
        if (p.at(0) == QLatin1Char('$')) {
            capture = p.mid(1).toInt();

            if (capture > max_capture) {
                max_capture = capture;
            }
        }
    }

    return max_capture;
}

int PatternMatcher::matchPattern(int first_term_index,
                                 QList<Baloo::Term> &matched_terms,
                                 int &start_position,
                                 int &end_position) const
{
    Q_ASSERT(first_term_index < terms.count());

    int pattern_index = 0;
    int term_index = first_term_index;
    bool has_matched_a_literal = false;
    bool match_anything = false;        // Match "$$"
    bool contains_catchall = false;

    start_position = 1 << 30;
    end_position = 0;

    while (pattern_index < pattern.count() && term_index < terms.count()) {
        const Baloo::Term &term = terms.at(term_index);
        int capture_index = -1;

        // Always update start and end position, they will be simply discarded
        // if the pattern ends not matching.
        start_position = qMin(start_position, termStart(term));
        end_position = qMax(end_position, termEnd(term));

        if (pattern.at(pattern_index) == QLatin1String("$$")) {
            // Start to match anything
            match_anything = true;
            contains_catchall = true;
            ++pattern_index;

            continue;
        }

        bool match = matchTerm(term, pattern.at(pattern_index), capture_index);

        if (match_anything) {
            if (!match) {
                // The stop pattern is not yet matched, continue to match
                // anything
                matched_terms.append(term);
            } else {
                // The terminating pattern is matched, stop to match anything
                match_anything = false;
                ++pattern_index;
            }
        } else if (match) {
            if (capture_index != -1) {
                matched_terms[capture_index] = term;
            } else {
                has_matched_a_literal = true;   // At least one literal has been matched, enable auto-completion
            }

            // Try to match the next pattern
            ++pattern_index;
        } else {
            // The pattern does not match, abort
            break;
        }

        // Match the next term
        ++term_index;
    }

    // See if the partially matched pattern can be used to provide a completion proposal
    if ((has_matched_a_literal || completion_type == Baloo::CompletionProposal::PropertyName) &&
        term_index - first_term_index > 0) {
        addCompletionProposal(pattern_index, first_term_index, term_index);
    }

    if (contains_catchall || pattern_index == pattern.count()) {
        // Patterns containing "$$" typically end with an optional terminating
        // term. Allow them to match even if we reach the end of the term list
        // without encountering the terminating term.
        return (term_index - first_term_index);
    } else {
        return 0;
    }
}

bool PatternMatcher::matchTerm(const Baloo::Term &term, const QString &pattern, int &capture_index) const
{
    if (pattern.at(0) == QLatin1Char('$')) {
        // Placeholder
        capture_index = pattern.mid(1).toInt() - 1;

        return true;
    } else {
        // Literal value that has to be matched against a regular expression
        QString value = stringValueIfLiteral(term);
        QStringList allowed_values = pattern.split(QLatin1Char('|'));

        if (value.isNull()) {
            return false;
        }

        Q_FOREACH(const QString &allowed_value, allowed_values) {
            if (QRegExp(allowed_value, Qt::CaseInsensitive, QRegExp::RegExp2).exactMatch(value)) {
                return true;
            }
        }

        return false;
    }
}

void PatternMatcher::addCompletionProposal(int first_pattern_index_not_matching,
                                           int first_term_index_matching,
                                           int first_term_index_not_matching) const
{
    // Don't count terms that are not literal terms. This avoids problems when the
    // user types "sent to size > 2M", that is seen here as "sent to <comparison>".
    if (!terms.at(first_term_index_not_matching - 1).property().isNull()) {
        if (--first_term_index_not_matching <= 0) {
            return; // Avoid an underflow when the pattern is only "$1" for instance.
        }
    }

    const Baloo::Term &first_matching = terms.at(first_term_index_matching);
    const Baloo::Term &last_matching = terms.at(first_term_index_not_matching - 1);

    // Check that the completion proposal would be valid
    if (completion_description.isEmpty()) {
        return;
    }

    if (cursor_position < termStart(first_matching)) {
        return;
    }

    if (first_term_index_not_matching < terms.count() &&
        cursor_position > termStart(terms.at(first_term_index_not_matching))) {
        return;
    }

    // Replace pattern parts that have matched with their matched terms, and
    // replace "a|b|c" with "a". This makes the pattern nicer for the user
    int pattern_parts_to_replace = first_term_index_not_matching - first_term_index_matching;
    QStringList user_friendly_pattern(pattern);

    for (int i=0; i<user_friendly_pattern.count(); ++i) {
        QString &part = user_friendly_pattern[i];

        if (part == QLatin1String("$$")) {
            // pattern_parts_to_replace and i cannot be used as $$ can hide
            // many terms matched by the pattern. Don't touch the pattern.
            break;
        } else if (part.startsWith(QLatin1Char('$'))) {
            continue;
        }

        if (i < pattern_parts_to_replace) {
            part = terms.at(first_term_index_matching + i).value().toString();
        } else {
            part = part.section(QLatin1Char('|'), 0, 0);
        }
    }

    parser->addCompletionProposal(new Baloo::CompletionProposal(
        user_friendly_pattern,
        first_pattern_index_not_matching - 1,
        termStart(first_matching),
        termEnd(last_matching) - termStart(first_matching) + 1,
        completion_type,
        completion_description
    ));
}
