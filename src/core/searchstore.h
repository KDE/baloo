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

#ifndef SEARCHSTORE_H
#define SEARCHSTORE_H

#include <QFlag>

class Type;

class SearchStore
{
public:
    SearchStore();
    virtual ~SearchStore();

    /**
     * Returns a list of types which can be searched for
     * in this store
     */
    QList<Type> types();

    enum Feature {
        /**
         * This search store only provides the most basic features
         */
        FeatureNone,

        /**
         * This Search Store provides a list of types which never change
         */
        FeatureStaticTypes,
        /**
         * The Search Store provides realtime notifications for when
         * a new type is stored and removed in the store
         */
        FeatureNewTypeNotification,

        /**
         * The Search Store provides a notification signifying that
         * the list of types supported by this store has changed
         */
        FeatureTypeChangeNotification
    };
    Q_DECLARE_ENUM(Features, Feature);
    //Q_FLAGS(Features);

    // Functions that would actually implement the searching!
signals:
    void typeAdded(const Type& type);
    void typeRemoved(const Type& type);

    void typesChanged();
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(SearchStore::Features);

#endif // SEARCHSTORE_H
