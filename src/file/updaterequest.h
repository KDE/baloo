/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NEPOMUK_UPDATE_REQUEST_H_
#define _NEPOMUK_UPDATE_REQUEST_H_

#include <KUrl>
#include <QtCore/QDateTime>

namespace Nepomuk2
{
/**
 * One update request with a timestamp.
 */
class UpdateRequest
{
public:
    UpdateRequest(const KUrl& s = KUrl(),
                  const KUrl& t = KUrl())
        : m_source(s),
          m_target(t) {
        m_timestamp = QDateTime::currentDateTime();
    }

    /// here the timestamp is ignored deliberately
    bool operator==(const UpdateRequest& other) const {
        return m_source.equals(other.m_source) && m_target.equals(other.m_target);
    }

    QDateTime timestamp() const {
        return m_timestamp;
    }
    KUrl source() const {
        return m_source;
    }
    KUrl target() const {
        return m_target;
    }

private:
    KUrl m_source;
    KUrl m_target;
    QDateTime m_timestamp;
};

uint qHash(const UpdateRequest& req);
}

#endif
