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

#ifndef TAGSTORE_H
#define TAGSTORE_H

#include "datastore.h"
#include "tag_export.h"
#include "tagfetchjob.h"

namespace Baloo {

class Tag;

class BALOO_TAG_EXPORT TagStore : public DataStore
{
    Q_OBJECT
public:
    static TagStore* instance();
    virtual ~TagStore();

    virtual TagFetchJob* fetchAll();
    virtual bool supportsWatching() const { return true; }

signals:
    void tagCreated(const Baloo::Tag& tag);
    void tagRemoved(const Baloo::Tag& tag);
    void tagModified(const Baloo::Tag& tag);

private slots:
    void slotWatchStatusChanged(bool status);
    void slotCreated(const QByteArray& id, const QString& name);
    void slotRemoved(const QByteArray& id);
    void slotModified(const QByteArray& id, const QString& name);

private:
    explicit TagStore(QObject* parent = 0);
};

}

#endif // TAGSTORE_H
