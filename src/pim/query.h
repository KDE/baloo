/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QUERY_H
#define QUERY_H

#include "pim_export.h"

#include <QStringList>
#include <Akonadi/Collection>

class QueryIterator;

class VIZIER_PIM_EXPORT Query
{
public:
    Query();

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

    void setImportant(bool important = true);
    void setRead(bool read = true);
    void setAttachment(bool hasAttachment = true);

    /**
     * Matches the string \p match anywhere in the entire email
     * body
     */
    void matches(const QString& match);

    void bodyMatches(const QString& match);

    /**
     * Matches teh string \p subjectMatch specifically in the
     * email subject
     */
    // FIXME: Is this really required?
    void subjectMatches(const QString& subjectMatch);

    void setLimit(int limit);
    int limit();

    /**
     * Execute the query and return an iterator to fetch
     * the results
     */
    QueryIterator exec();

private:
    QString m_path;

    QStringList m_involves;
    QStringList m_to;
    QStringList m_cc;
    QStringList m_bcc;
    QString m_from;

    QList<Akonadi::Collection::Id> m_collections;

    bool m_important;
    bool m_read;
    bool m_attachment;

    QString m_matchString;
    QString m_bodyMatchString;
    QString m_subjectMatchString;

    int m_limit;
};

#endif // QUERY_H
