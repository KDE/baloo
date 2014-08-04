/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _EMAIL_QUERY_H
#define _EMAIL_QUERY_H

#include "pim_export.h"
#include "query.h"

#include <QStringList>
#include <Collection>

namespace Baloo {
namespace PIM {

class BALOO_PIM_EXPORT EmailQuery : public Query
{
public:
    EmailQuery();
    virtual ~EmailQuery();

    enum OpType {
        OpAnd = 0,
        OpOr
    };

    void setSplitSearchMatchString(bool split);

    void setSearchType(OpType op);

    void setInvolves(const QStringList& involves);
    void addInvolves(const QString& email);

    void setTo(const QStringList& to);
    void addTo(const QString& to);

    void setFrom(const QString& from);
    void addFrom(const QString& from);

    void setCc(const QStringList& cc);
    void addCc(const QString& cc);

    void setBcc(const QStringList& bcc);
    void addBcc(const QString& bcc);

    void setCollection(const QList<Akonadi::Collection::Id>& collections);
    void addCollection(Akonadi::Collection::Id id);

    /**
     * By default the importance is ignored
     */
    void setImportant(bool important = true);

    /**
     * By default the read status is ignored
     */
    void setRead(bool read = true);

    /**
     * By default the attachment status is ignored
     */
    void setAttachment(bool hasAttachment = true);

    /**
     * Matches the string \p match anywhere in the entire email
     * body
     */
    void matches(const QString& match);

    /**
     * Matches the string \p subjectMatch specifically in the
     * email subject
     */
    void subjectMatches(const QString& subjectMatch);

    /**
     * Matches the string \p bodyMatch specifically in the body email
     */
    void bodyMatches(const QString& bodyMatch);


    void setLimit(int limit);
    int limit() const;

    /**
     * Execute the query and return an iterator to fetch
     * the results
     */
    ResultIterator exec();

private:
    //@cond PRIVATE
    class Private;
    Private * const d;
    //@endcond
};

}
}

#endif // _EMAIL_QUERY_H
