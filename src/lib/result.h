/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef _BALOO_CORE_RESULT_H
#define _BALOO_CORE_RESULT_H

#include <QString>
#include <QByteArray>
#include <QUrl>

#include "core_export.h"

namespace Baloo {

class BALOO_CORE_EXPORT Result
{
public:
    Result();
    Result(const Result& rhs);
    ~Result();

    QByteArray id() const;
    void setId(const QByteArray& id);

    /**
     * Returns an icon that could be used when displaying
     * the result.
     *
     * Most often there is no icon
     */
    QString icon() const;
    void setIcon(const QString& icon);

    /**
     * Represents the url of the item returned. The item
     * may not always contain a url
     */
    QUrl url() const;
    void setUrl(const QUrl& url);

    Result& operator=(const Result& rhs);

private:
    class Private;
    Private* d;
};

}

#endif // _BALOO_CORE_RESULT_H
