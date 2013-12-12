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

#include "kio_timeline.h"
#include "timelinetools.h"
#include "query.h"
#include "resultiterator.h"

#include <KUrl>
#include <kio/global.h>
#include <klocale.h>
#include <kio/job.h>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <kio/netaccess.h>
#include <KComponentData>
#include <KCalendarSystem>
#include <kde_file.h>

#include <QDate>
#include <QCoreApplication>

using namespace Baloo;

namespace
{
KIO::UDSEntry createFolderUDSEntry(const QString& name, const QString& displayName, const QDate& date)
{
    KIO::UDSEntry uds;
    QDateTime dt(date, QTime(0, 0, 0));
    uds.insert(KIO::UDSEntry::UDS_NAME, name);
    uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayName);
    uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
    uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, dt.toTime_t());
    uds.insert(KIO::UDSEntry::UDS_CREATION_TIME, dt.toTime_t());
    uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    return uds;
}

KIO::UDSEntry createMonthUDSEntry(int month, int year)
{
    QString dateString
        = KGlobal::locale()->calendar()->formatDate(QDate(year, month, 1),
                i18nc("Month and year used in a tree above the actual days. "
                      "Have a look at http://api.kde.org/4.x-api/kdelibs-"
                      "apidocs/kdecore/html/classKCalendarSystem.html#a560204439a4b670ad36c16c404f292b4 "
                      "to see which variables you can use and ask kde-i18n-doc@kde.org if you have "
                      "problems understanding how to translate this",
                      "%B %Y"));
    return createFolderUDSEntry(QDate(year, month, 1).toString(QLatin1String("yyyy-MM")),
                                dateString,
                                QDate(year, month, 1));
}

KIO::UDSEntry createDayUDSEntry(const QDate& date)
{
    KIO::UDSEntry uds = createFolderUDSEntry(date.toString("yyyy-MM-dd"),
                        KGlobal::locale()->formatDate(date, KLocale::FancyLongDate),
                        date);

    return uds;
}

KIO::UDSEntry createFileUDSEntry(const KUrl& fileUrl)
{
    KIO::UDSEntry uds;
    // Code from kdelibs/kioslaves/file.cpp
    KDE_struct_stat statBuf;
    if( KDE_stat(QFile::encodeName(fileUrl.toLocalFile()).data(), &statBuf ) == 0) {
        uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, statBuf.st_mtime);
        uds.insert(KIO::UDSEntry::UDS_ACCESS_TIME, statBuf.st_atime);
        uds.insert(KIO::UDSEntry::UDS_SIZE, statBuf.st_size);
        uds.insert(KIO::UDSEntry::UDS_USER, statBuf.st_uid);
        uds.insert(KIO::UDSEntry::UDS_GROUP, statBuf.st_gid);

        mode_t type = statBuf.st_mode & S_IFMT;
        mode_t access = statBuf.st_mode & 07777;

        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, type);
        uds.insert(KIO::UDSEntry::UDS_ACCESS, access);
        uds.insert(KIO::UDSEntry::UDS_URL, fileUrl.url());
        uds.insert(KIO::UDSEntry::UDS_NAME, fileUrl.fileName());
    }

    return uds;
}

}


TimelineProtocol::TimelineProtocol(const QByteArray& poolSocket, const QByteArray& appSocket)
    : KIO::SlaveBase("timeline", poolSocket, appSocket)
{
}


TimelineProtocol::~TimelineProtocol()
{
}


void TimelineProtocol::listDir(const KUrl& url)
{
    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder:
        listEntry(createFolderUDSEntry(QLatin1String("today"), i18n("Today"), QDate::currentDate()), false);
        listEntry(createFolderUDSEntry(QLatin1String("calendar"), i18n("Calendar"), QDate::currentDate()), false);
        listEntry(KIO::UDSEntry(), true);
        finished();
        break;

    case CalendarFolder:
        listThisYearsMonths();
        // TODO: add entry for previous years
        listEntry(KIO::UDSEntry(), true);
        finished();
        break;

    case MonthFolder:
        listDays(m_date.month(), m_date.year());
        listEntry(KIO::UDSEntry(), true);
        finished();
        break;

    case DayFolder: {
        Query query;
        query.addType("File");
        query.setDateFilter(m_date.year(), m_date.month(), m_date.day());

        ResultIterator it = query.exec();
        while (it.next()) {
            KIO::UDSEntry uds = createFileUDSEntry(it.url());
            if (uds.count())
                listEntry(uds, false);
        }
        listEntry(KIO::UDSEntry(), true);
        finished();
        break;
    }

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


void TimelineProtocol::mimetype(const KUrl& url)
{
    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder:
    case CalendarFolder:
    case MonthFolder:
    case DayFolder:
        mimetype(QString::fromLatin1("inode/directory"));
        break;

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


void TimelineProtocol::stat(const KUrl& url)
{
    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder: {
        KIO::UDSEntry uds;
        uds.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QString::fromLatin1("nepomuk"));
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
        statEntry(uds);
        finished();
        break;
    }

    case CalendarFolder:
        statEntry(createFolderUDSEntry(QLatin1String("calendar"), i18n("Calendar"), QDate::currentDate()));
        finished();
        break;

    case MonthFolder:
        statEntry(createMonthUDSEntry(m_date.month(), m_date.year()));
        finished();
        break;

    case DayFolder:
        if (m_filename.isEmpty()) {
            statEntry(createDayUDSEntry(m_date));
            finished();
        }
        break;

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


void TimelineProtocol::listDays(int month, int year)
{
    const int days = KGlobal::locale()->calendar()->daysInMonth(year, month);
    for (int day = 1; day <= days; day++) {
        QDate date(year, month, day);

        if (date <= QDate::currentDate() && filesInDate(date)) {
            listEntry(createDayUDSEntry(date), false);
        }
    }
}

bool TimelineProtocol::filesInDate(const QDate& date)
{
    Query query;
    query.addType("File");
    query.setLimit(1);
    query.setDateFilter(date.year(), date.month(), date.day());

    ResultIterator it = query.exec();
    return it.next();
}


void TimelineProtocol::listThisYearsMonths()
{
    Query query;
    query.addType("File");
    query.setLimit(1);

    int year = QDate::currentDate().year();
    int currentMonth = QDate::currentDate().month();
    for (int month = 1; month <= currentMonth; ++month) {
        query.setDateFilter(year, month);
        ResultIterator it = query.exec();
        if (it.next()) {
            listEntry(createMonthUDSEntry(month, year), false);
        }
    }
}


extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // necessary to use other kio slaves
        KComponentData("kio_timeline");
        QCoreApplication app(argc, argv);

        kDebug() << "Starting timeline slave " << getpid();

        if (argc != 4) {
            kError() << "Usage: kio_timeline protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        Baloo::TimelineProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug() << "Timeline slave Done";

        return 0;
    }
}
