/*
   Copyright 2009-2010 Sebastian Trueg <trueg@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _KIO_TIMELINE_H_
#define _KIO_TIMELINE_H_

#include <kio/slavebase.h>

#include <QtCore/QDate>
#include "src/file/database.h"

namespace Baloo
{

class TimelineProtocol : public KIO::SlaveBase
{
public:
    TimelineProtocol(const QByteArray& poolSocket, const QByteArray& appSocket);
    virtual ~TimelineProtocol();

    /**
     * List all files and folders tagged with the corresponding tag.
     */
    void listDir(const KUrl& url);

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void mimetype(const KUrl& url);

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    void stat(const KUrl& url);

private:
    void listDays(int month, int year);
    void listThisYearsMonths();
    void listPreviousYears();

    /// temp vars for the currently handled URL
    QDate m_date;
    QString m_filename;

    Database* m_db;
};
}

#endif // _KIO_TIMELINE_H_
