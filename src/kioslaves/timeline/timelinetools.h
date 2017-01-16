/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef BALOO_TIMELINE_TOOLS_H_
#define BALOO_TIMELINE_TOOLS_H_

class QString;

#include <QDate>
#include <QUrl>

namespace Baloo
{
/**
 * The hierarchy in timeline:/ looks as follows:
 *
 * /
 * |- /today
 * |
 * |- /calendar
 * |  |- /calendar/2010
 * |     |- /calendar/2010/january
 * |        |- /calendar/2010/january/2010-01-01
 * |  |- /calendar/2009
 * |
 * |- /months
 * |  |- /month/2010
 * |     |- /months/2010/january
 * |
 * |- /weeks
 */
enum TimelineFolderType {
    NoFolder = 0,    /// nothing
    RootFolder,      /// the root folder
    CalendarFolder,  /// the calendar folder listing all months
    MonthFolder,     /// a folder listing a month's days (m_date contains the month)
    DayFolder        /// a folder listing a day (m_date); optionally m_filename is set
};

/**
 * Parse a timeline URL like timeline:/today and return the type of folder it
 * represents. If DayFolder is returned \p date is set to the date that should be listed.
 * Otherwise it is an invalid date. \p filename is optionally set to the name of the file
 * in the folder.
 */
TimelineFolderType parseTimelineUrl(const QUrl& url, QDate* date, QString* filename = nullptr);

}

#endif
