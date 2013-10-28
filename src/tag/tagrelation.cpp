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

#include "tagrelation.h"
#include "item.h"
#include "tag.h"

using namespace Baloo;

TagRelation::TagRelation()
{
}

TagRelation::TagRelation(const Tag& tag, const Item& item)
    : m_tag(tag)
    , m_item(item)
{
}

TagRelation::TagRelation(const Tag& tag)
    : m_tag(tag)
{
}

TagRelation::TagRelation(const Item& item)
    : m_item(item)
{
}

Tag& TagRelation::tag()
{
    return m_tag;
}

Item& TagRelation::item()
{
    return m_item;
}

const Tag& TagRelation::tag() const
{
    return m_tag;
}

const Item& TagRelation::item() const
{
    return m_item;
}

void TagRelation::setItem(const Item& item)
{
    m_item = item;
}

void TagRelation::setTag(const Tag& tag)
{
    m_tag = tag;
}

TagRelationCreateJob* TagRelation::create()
{
    return new TagRelationCreateJob(*this);
}

TagRelationFetchJob* TagRelation::fetch()
{
    return new TagRelationFetchJob(*this);
}

TagRelationRemoveJob* TagRelation::remove()
{
    return new TagRelationRemoveJob(*this);
}
