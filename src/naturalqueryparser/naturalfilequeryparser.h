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

#ifndef _BALOO_NATURAL_FILE_QUERY_PARSER_H_
#define _BALOO_NATURAL_FILE_QUERY_PARSER_H_

#include "naturalqueryparser.h"

namespace Baloo {
/**
 * \class NaturalFileQueryParser naturalqueryparser.h baloo/naturalqueryparser.h
 *
 * \brief Parser for desktop file user queries.
 *
 * Natural query parser tailored for File queries. It extends NaturalQueryParser
 * (please read its documentation for advice and important notices) by providing
 * patterns specifically targetting file queries. For instance, the user can
 * specify which type a file must have, or what its size should be.
 */
class BALOO_NATURALQUERYPARSER_EXPORT NaturalFileQueryParser : public NaturalQueryParser
{
public:
    /**
     * Create a new query parser.
     */
    NaturalFileQueryParser();

    /**
     * Destructor
     */
    ~NaturalFileQueryParser();

    /**
     * Convenience method to quickly parse a query without creating an object.
     *
     * \warning The parser caches many useful information that is slow to
     *          retrieve from the Baloo database. If you have to parse
     *          many queries, consider using a QueryParser object.
     *
     * \param query The query string to parse
     * \param flags a set of flags influencing the parsing process.
     * \return The parsed query or an invalid Query object in case the parsing failed.
     */
    static Query parseQuery( const QString& query, ParserFlags flags = NoParserFlags);

protected:
    virtual void addSpecificPatterns(int cursor_position, NaturalQueryParser::ParserFlags flags) const;

private:
    struct Private;
    Private *d;
};

}

#endif
