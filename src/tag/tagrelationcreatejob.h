/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#ifndef TAGRELATIONCREATEJOB_H
#define TAGRELATIONCREATEJOB_H

#include "relationcreatejob.h"
#include "tag_export.h"

namespace Baloo {

class Item;
class Tag;
class TagRelation;

class BALOO_TAG_EXPORT TagRelationCreateJob : public RelationCreateJob
{
    Q_OBJECT
public:
    TagRelationCreateJob(const TagRelation& relation, QObject* parent = 0);

    /**
     * Assigns \p tags to \p items. If some of those items already contain
     * those tags, then those relations are just ignored.
     */
    TagRelationCreateJob(const QList<Item>& items, const QList<Tag>& tags, QObject* parent = 0);
    ~TagRelationCreateJob();

    virtual void start();

    enum Errors {
        Error_InvalidRelation,
        Error_RelationExists,
        Error_InvalidTagId,
        Error_ConnectionError
    };
Q_SIGNALS:
    void tagRelationCreated(const Baloo::TagRelation& relation);

private Q_SLOTS:
    void doStart();
    void slotTagFetched(const Baloo::Tag& tag);
    void slotTagFetchJobFinished(KJob* job);

private:
    class Private;
    Private* d;
};

}

#endif // TAGRELATIONCREATEJOB_H
