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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>
#include <QVariant>

/**
 * It would be best not to derive from this class
 */
class Property
{
public:
    Property();
    virtual ~Property();

    virtual QString name();
    //virtual void setName(const QString& name);

    virtual QVariant::Type valueType();
    //virtual void setValueType(const QVariant::Type& valueType);

    /**
     * Random ID generated at runtime. This ID is not stable
     * and will change each time. It can be used as an efficient
     * way to refer to a Property.
     */
    int id();

    /**
     * Indicates a property who value should never be changed as
     * it will never be saved. This typically applies to properties
     * that are indexed such as FileIndex and Email Index
     *
     * These properties should be directly editing on the source,
     * and then reindexed
     */
    virtual bool readOnly();
};

// Declare a property
#define V_DECL_PROPERTY(ClassName, translatableName, valueType_, isReadOnly) \
    class ClassName : public Property { \
    public: \
        QString name() { return #ClassName; } \
    };

#define blah
    Type get

#define V_DECL_PROPERY_RO(ClassName, translatableName, valueType_) V_DECL_PROPERTY(ClassName, translatableName, valueType_, true)
#define V_DECL_PROPERY_RW(ClassName, translatableName, valueType_) V_DECL_PROPERTY(ClassName, translatableName, valueType_, false)


V_DECL_PROPERTY_RO(Artist); // String
V_DECL_PROPERTY_RO(Album);
V_DECL_PROPERTY_RO(AlbumArtist);
V_DECL_PROPERTY_RO(Title);


iten.property(Artist(), dafsdf)
#endif // PROPERTY_H
