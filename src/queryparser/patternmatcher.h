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

#ifndef __PATTERNMATCHER_H__
#define __PATTERNMATCHER_H__

#include "completionproposal.h"
#include "utils.h"

#include "term.h"

#include <QtCore/QStringList>

namespace Baloo { class QueryParser; }

class PatternMatcher
{
    public:
        PatternMatcher(Baloo::QueryParser *parser,
                       QList<Baloo::Term> &terms,
                       int cursor_position,
                       const QStringList &pattern,
                       Baloo::CompletionProposal::Type completion_type,
                       const KLocalizedString &completion_description);

        template<typename T>
        void runPass(const T &pass)
        {
            QList<Baloo::Term> matched_terms;

            for (int i=0; i<capture_count; ++i) {
                matched_terms.append(Baloo::Term());
            }

            // Try to start to match the pattern at every position in the term list
            for (int index=0; index<terms.count(); ++index) {
                int start_position;
                int end_position;
                int matched_length = matchPattern(index, matched_terms, start_position, end_position);

                if (matched_length > 0) {
                    // The pattern matched, run the pass on the matching terms
                    QList<Baloo::Term> replacement = pass.run(matched_terms);

                    if (replacement.count() > 0) {
                        // Replace terms first_match_index..i with replacement
                        for (int i=0; i<matched_length; ++i) {
                            terms.removeAt(index);
                        }

                        for (int i=replacement.count()-1; i>=0; --i) {
                            terms.insert(index, replacement.at(i));
                        }

                        // If the pass returned only one replacement term, set
                        // its position. If more terms are returned, the pass
                        // must handle positions itself
                        if (replacement.count() == 1) {
                            setTermRange(terms[index], start_position, end_position);
                        }

                        // Re-explore the terms vector as indexes have changed
                        index = -1;
                    }

                    // If the pattern contains "$$", terms are appended to the end
                    // of matched_terms. Remove them now that they are not needed anymore
                    while (matched_terms.count() > capture_count) {
                        matched_terms.removeLast();
                    }
                }
            }
        }

    private:
        int captureCount() const;
        int matchPattern(int first_term_index,
                         QList<Baloo::Term> &matched_terms,
                         int &start_position,
                         int &end_position) const;
        bool matchTerm(const Baloo::Term &term, const QString &pattern, int &capture_index) const;
        void addCompletionProposal(int first_pattern_index_not_matching,
                                   int first_term_index_matching,
                                   int first_term_index_not_matching) const;

    private:
        Baloo::QueryParser *parser;
        QList<Baloo::Term> &terms;
        int cursor_position;
        QStringList pattern;
        Baloo::CompletionProposal::Type completion_type;
        KLocalizedString completion_description;

        int capture_count;
};

#endif
