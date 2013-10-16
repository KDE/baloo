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

#ifndef TAGRELATION_H
#define TAGRELATION_H

#include "relation.h"
#include "tag.h"
#include "tagrelationfetchjob.h"
#include "tagrelationcreatejob.h"
#include "tagrelationremovejob.h"

class VIZIER_TAG_EXPORT TagRelation : public Relation
{
public:
    TagRelation();
    TagRelation(const Tag& tag);
    TagRelation(const Item& item);
    TagRelation(const Tag& tag, const Item& item);

    Tag& tag();
    Item& item();

    const Tag& tag() const;
    const Item& item() const;

    void setTag(const Tag& tag);
    void setItem(const Item& item);

    QByteArray fromType() const { return "Tag"; }
    QByteArray toType() const { return "Item"; }

    TagRelationFetchJob* fetch();
    TagRelationCreateJob* create();
    TagRelationRemoveJob* remove();

private:
    Tag m_tag;
    Item m_item;
};

Q_DECLARE_METATYPE(TagRelation);

#endif // TAGRELATION_H
