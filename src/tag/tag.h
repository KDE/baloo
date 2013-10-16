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

#ifndef TAG_H
#define TAG_H

#include "item.h"
#include "tagfetchjob.h"
#include "tagcreatejob.h"
#include "tagsavejob.h"
#include "tagremovejob.h"

class VIZIER_TAG_EXPORT Tag : public Item
{
public:
    Tag();
    Tag(const QString& name);

    static Tag fromId(const Item::Id& id);

    QByteArray type();

    QString name() const;
    void setName(const QString& name);

    TagFetchJob* fetch();
    TagSaveJob* save();
    TagCreateJob* create();
    TagRemoveJob* remove();

private:
    QString m_name;
};

//Q_DECLARE_METATYPE(Tag);
Q_DECLARE_METATYPE(Tag*);

#endif // TAG_H
