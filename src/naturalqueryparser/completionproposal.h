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

#ifndef __BALOO_SEARCH_COMPLETION_PROPOSAL_H__
#define __BALOO_SEARCH_COMPLETION_PROPOSAL_H__

#include <QtCore/QtGlobal>
#include <QtCore/QStringList>

#include "naturalqueryparser_export.h"
#include "klocalizedstring.h"

namespace Baloo
{

/**
 * \class CompletionProposal completionproposal.h Baloo/Query/CompletionProposal
 * \brief Information about an auto-completion proposal
 *
 * When parsing an user query, QueryParser may find that a pattern that nearly
 * matches and that the user may want to use. In this case, one or more
 * completion proposals are used to describe what patterns can be used.
 */
class BALOO_NATURALQUERYPARSER_EXPORT CompletionProposal
{
    private:
        Q_DISABLE_COPY(CompletionProposal)

    public:
        /**
         * \brief Data-type used by the first placeholder of the pattern
         *
         * If the pattern is "sent by $1", the type of "$1" is Contact. This
         * way, a GUI can show to the user a list of his or her contacts.
         */
        enum Type
        {
            NoType,         /*!< No specific type (integer, string, something that does not need any auto-completion list) */
            DateTime,       /*!< A date-time */
            Tag,            /*!< A valid tag name */
            Contact,        /*!< Something that can be parsed unambiguously to a contact (a contact name, email, pseudo, etc) */
            Email,          /*!< An e-mail address */
            PropertyName,   /*!< A property name. List of these names can be found in kde:kfilemetadata/src/propertyinfo.h */
        };

        /**
         * \param pattern list of terms matched by the proposal ("sent",
         *                "by", "$1" for instance)
         * \param last_matched_part index of the last part of the mattern
         *                          that has been matched against the user
         *                          query
         * \param position position in the user query of the pattern matched
         * \param length length in the user query of the terms matched
         * \param type if the pattern contains "$1", this is the type of
         *             the value matched by this placeholder
         * \param description human description of the pattern
         */
        CompletionProposal(const QStringList &pattern,
                            int last_matched_part,
                            int position,
                            int length,
                            Type type,
                            const KLocalizedString &description);
        ~CompletionProposal();

        /**
         * \return list of terms that make the pattern
         */
        QStringList pattern() const;

        /**
         * \return index of the last matched part of the pattern
         */
        int lastMatchedPart() const;

        /**
         * \return position in the user query of the pattern
         *
         * As an user query can contain spaces and separators that are
         * ignored by the pattern matcher, position() and length() are
         * used to find the sub-string of the user query that has matched
         * against the pattern.
         */
        int position() const;

        /**
         * \sa position
         */
        int length() const;

        /**
         * \return type of the value represented by the "$1" term in the pattern
         */
        Type type() const;

        /**
         * \return description of the pattern
         */
        KLocalizedString description() const;

    private:
        struct Private;
        Private *const d;
};

}

#endif
