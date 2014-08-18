/*
   This file is part of the Baloo KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef _BALOO_NATURAL_QUERY_PARSER_H_
#define _BALOO_NATURAL_QUERY_PARSER_H_

#include "query.h"
#include "completionproposal.h"
#include "naturalqueryparser_export.h"

#include <QtCore/QString>

class PatternMatcher;
class PassProperties;

namespace Baloo {
/**
 * \class NaturalQueryParser naturalqueryparser.h baloo/naturalqueryparser.h
 *
 * \brief Parser for desktop user queries.
 *
 * \warning This is NOT a SPARQL parser.
 * \note Don't forget to insert the "baloo_naturalqueryparser" localization catalog
 *       for this class to be localized. Do so using KLocale::insertCatalog.
 *
 * The NaturalQueryParser can be used to parse user queries, ie. queries that the user
 * would enter in any search interface, and convert them into Query instances.
 *
 * The syntax is language-dependent and as natural as possible. Words are
 * by default matched against the content of documents, and special patterns
 * like "sent by X" are recognized and changed to property comparisons.
 *
 * Natural date-times can also be used and are recognized as date-time
 * literals. "sent on March 6" and "created last week" work as expected.
 *
 * \section naturalqueryparser_examples Examples
 *
 * %Query everything that contains the term "baloo":
 * \code
 * baloo
 * \endcode
 *
 * %Query everything that contains both the terms "Hello" and "World":
 * \code
 * Hello World
 * \endcode
 *
 * %Query everything that contains the term "Hello World":
 * \code
 * "Hello World"
 * \endcode
 *
 * %Query everything that has a tag whose label matches "baloo":
 * \code
 * tagged as baloo, has tag baloo
 * \endcode
 *
 * %Query everything that has a tag labeled "baloo" or a tag labeled "scribo":
 * \code
 * tagged as baloo OR tagged as scribo
 * \endcode
 *
 * \note "tagged as baloo or scribo" currently does not work as expected
 *
 * %Query everything that has a tag labeled "baloo" but not a tag labeled "scribo":
 * \code
 * tagged as baloo and not tagged as scribo
 * \endcode
 *
 * Some properties also accept nested queries. For instance, this %Query
 * returns the list of documents related to emails tagged as important.
 * \code
 * documents related to mails tagged as important
 * \endcode
 *
 * More complex nested queries can be built, and are ended with a comma
 * (in English):
 * \code
 * documents related to mails from Smith and tagged as important, size > 2M
 * \endcode
 *
 * \author Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * \since 4.14
 */
class BALOO_NATURALQUERYPARSER_EXPORT NaturalQueryParser
{
    friend class ::PatternMatcher;

public:
    /**
     * Create a new query parser.
     */
    NaturalQueryParser();

    /**
     * Destructor
     */
    virtual ~NaturalQueryParser();

    /**
     * Flags to change the behaviour of the parser.
     */
    enum ParserFlag {
        /**
         * No flags. Default for parse()
         */
        NoParserFlags = 0x0,

        /**
         * Try to detect filename pattern like *.mp3
         * or hello*.txt and use regular expressions to match these terms
         */
        DetectFilenamePattern = 0x1
    };
    Q_DECLARE_FLAGS( ParserFlags, ParserFlag )

    /**
     * Parse a user query.
     *
     * \param query The query string to parse
     * \param flags a set of flags influencing the parsing process.
     * \param cursor_position position of the cursor in a line edit used
     *                        by the user to enter the query. It is used
     *                        to provide auto-completion proposals.
     *
     * \return The parsed query or an invalid Query object
     * in case the parsing failed.
     */
    Query parse(const QString& query,
                ParserFlags flags = NoParserFlags,
                int cursor_position = -1) const;

    /**
     * List of completion proposals related to the previously parsed query
     *
     * \note The query parser is responsible for deleting the CompletionProposal
     *       objects.
     */
    QList<CompletionProposal *> completionProposals() const;

private:
    void addCompletionProposal(CompletionProposal *proposal) const;
    QStringList split(const QString &query, bool is_user_query, QList<int> *positions = NULL) const;
    QList<Term> &terms() const;

protected:
    virtual void addSpecificPatterns(int cursor_position, NaturalQueryParser::ParserFlags flags) const;

    template<typename T>
    void runPass(const T &pass,
                 int cursor_position,
                 const QString &pattern,
                 const KLocalizedString &description = KLocalizedString(),
                 CompletionProposal::Type type = CompletionProposal::NoType) const;

    // Some passes that subclass may want to use
    PassProperties &passProperties() const;

private:
    struct Private;
    Private* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::NaturalQueryParser::ParserFlags)

#endif
