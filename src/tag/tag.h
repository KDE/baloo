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

#ifndef TAG_H
#define TAG_H

#include "item.h"
#include "tagfetchjob.h"
#include "tagcreatejob.h"
#include "tagsavejob.h"
#include "tagremovejob.h"

namespace Baloo {

class BALOO_TAG_EXPORT Tag : public Item
{
public:
    Tag();
    Tag(const QString& name);

    static Tag fromId(const Item::Id& id);

    QByteArray type();

    QString name() const;
    void setName(const QString& name);

private:
    QString m_name;
};

inline bool operator ==(const Tag& t1, const Tag& t2)
{
    if (t1.id().size() && t2.id().size())
        return t1.id() == t2.id();

    if (t1.name().size() && t2.name().size())
        return t1.name() == t2.name();

    return false;
}

}

Q_DECLARE_METATYPE(Baloo::Tag);

#endif // TAG_H
