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

#ifndef _BALOO_SEARCH_QUERY_PARSER_H_
#define _BALOO_SEARCH_QUERY_PARSER_H_

#include "query.h"
#include "core_export.h"

#include <QtCore/QString>

class PatternMatcher;

namespace Baloo {
class CompletionProposal;

/**
 * \class QueryParser queryparser.h baloo/queryparser.h
 *
 * \brief Parser for desktop user queries.
 *
 * \warning This is NOT a SPARQL parser.
 * \note Don't forget to insert the "libbaloocore" localization catalog
 *       for this class to be localized. Do so using KLocale::insertCatalog.
 *
 * The QueryParser can be used to parse user queries, ie. queries that the user
 * would enter in any search interface, and convert them into Query instances.
 *
 * The syntax is language-dependent and as natural as possible. Words are
 * by default matched against the content of documents, and special patterns
 * like "sent by X" are recognized and changed to property comparisons.
 *
 * Natural date-times can also be used and are recognized as date-time
 * literals. "sent on March 6" and "created last week" work as expected.
 *
 * \section queryparser_examples Examples
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
class BALOO_CORE_EXPORT QueryParser
{
    friend class ::PatternMatcher;

public:
    /**
     * Create a new query parser.
     */
    QueryParser();

    /**
     * Destructor
     */
    ~QueryParser();

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
     * \return The parsed query or an invalid Query object
     * in case the parsing failed.
     */
    Query parse( const QString& query ) const;

    /**
     * Parse a user query.
     *
     * \param query The query string to parse
     * \param flags a set of flags influencing the parsing process.
     *
     * \return The parsed query or an invalid Query object
     * in case the parsing failed.
     */
    Query parse( const QString& query, ParserFlags flags ) const;

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
    Query parse( const QString& query, ParserFlags flags, int cursor_position ) const;

    /**
     * List of completion proposals related to the previously parsed query
     *
     * \note The query parser is responsible for deleting the CompletionProposal
     *       objects.
     */
    QList<CompletionProposal *> completionProposals() const;

    /**
     * Convenience method to quickly parse a query without creating an object.
     *
     * \warning The parser caches many useful information that is slow to
     *          retrieve from the Baloo database. If you have to parse
     *          many queries, consider using a QueryParser object.
     *
     * \return The parsed query or an invalid Query object
     * in case the parsing failed.
     */
    static Query parseQuery( const QString& query );

    /**
     * \overload
     *
     * \param query The query string to parse
     * \param flags a set of flags influencing the parsing process.
     */
    static Query parseQuery( const QString& query, ParserFlags flags );

private:
    void addCompletionProposal(CompletionProposal *proposal);

private:
    struct Private;
    Private* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Baloo::QueryParser::ParserFlags)

#endif
