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

#ifndef TAGREMOVEJOB_H
#define TAGREMOVEJOB_H

#include "itemremovejob.h"

namespace Baloo {

class Tag;

class TagRemoveJob : public ItemRemoveJob
{
    Q_OBJECT
public:
    TagRemoveJob(const Tag& tag, QObject* parent = 0);
    ~TagRemoveJob();

    virtual void start();

    enum Error {
        Error_ConnectionError,
        Error_TagInvalidId,
        Error_TagEmptyId,
        Error_TagDoesNotExist
    };

signals:
    void tagRemoved(const Baloo::Tag& tag);

private slots:
    void doStart();

private:
    class Private;
    Private* d;
};

}

#endif // TAGREMOVEJOB_H
