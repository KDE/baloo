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
#include "nepomukservicecontrolinterface.h"
#include "timelinetools.h"

#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Vocabulary/NFO>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NUAO>

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

#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Node>

#include <QtCore/QDate>
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include <sys/types.h>
#include <unistd.h>

using namespace KIO;


namespace
{
KIO::UDSEntry createFolderUDSEntry(const QString& name, const QString& displayName, const QDate& date)
{
    KIO::UDSEntry uds;
    QDateTime dt(date, QTime(0, 0, 0));
    kDebug() << dt;
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
    uds.insert(KIO::UDSEntry::UDS_NEPOMUK_QUERY, Nepomuk2::buildTimelineQuery(date).toString());
    return uds;
}

bool filesInDateRange(const QDate& from, const QDate& to = QDate())
{
    // We avoid inference over here since it speeds up the query by an order of ~110ms
    // Not sure why it happens, but it is worth it. Specially, since we are just listing
    // the nie:lastModified. We don't need inference
    return Nepomuk2::ResourceManager::instance()->mainModel()->executeQuery(
               Nepomuk2::buildTimelineQuery(from, to).toSparqlQuery(Nepomuk2::Query::Query::CreateAskQuery),
               Soprano::Query::QueryLanguageSparqlNoInference).boolValue();
}
}


Nepomuk2::TimelineProtocol::TimelineProtocol(const QByteArray& poolSocket, const QByteArray& appSocket)
    : KIO::ForwardingSlaveBase("timeline", poolSocket, appSocket)
{
    kDebug();
}


Nepomuk2::TimelineProtocol::~TimelineProtocol()
{
    kDebug();
}


void Nepomuk2::TimelineProtocol::listDir(const KUrl& url)
{
    // without a running file indexer timeline is not at all reliable
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.nepomuk.services.nepomukfileindexer") ||
            !org::kde::nepomuk::ServiceControl("org.kde.nepomuk.services.nepomukfileindexer",
                    "/servicecontrol",
                    QDBusConnection::sessionBus()).isInitialized()) {
        error(KIO::ERR_SLAVE_DEFINED,
              i18n("The file indexing service is not running. Without it timeline results are not available."));
        return;
    }

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

    case DayFolder:
        ForwardingSlaveBase::listDir(url);
        break;

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


void Nepomuk2::TimelineProtocol::mkdir(const KUrl& url, int permissions)
{
    Q_UNUSED(permissions);
    error(ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}


void Nepomuk2::TimelineProtocol::get(const KUrl& url)
{
    kDebug() << url;

    if (parseTimelineUrl(url, &m_date, &m_filename) && !m_filename.isEmpty()) {
        ForwardingSlaveBase::get(url);
    } else {
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
    }
}


void Nepomuk2::TimelineProtocol::put(const KUrl& url, int permissions, KIO::JobFlags flags)
{
    kDebug() << url;

    if (parseTimelineUrl(url, &m_date, &m_filename) && !m_filename.isEmpty()) {
        ForwardingSlaveBase::put(url, permissions, flags);
    } else {
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
    }
}


void Nepomuk2::TimelineProtocol::copy(const KUrl& src, const KUrl& dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(src);
    Q_UNUSED(dest);
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    error(ERR_UNSUPPORTED_ACTION, src.prettyUrl());
}


void Nepomuk2::TimelineProtocol::rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(src);
    Q_UNUSED(dest);
    Q_UNUSED(flags);

    error(ERR_UNSUPPORTED_ACTION, src.prettyUrl());
}


void Nepomuk2::TimelineProtocol::del(const KUrl& url, bool isfile)
{
    kDebug() << url;
    ForwardingSlaveBase::del(url, isfile);
}


void Nepomuk2::TimelineProtocol::mimetype(const KUrl& url)
{
    kDebug() << url;
    ForwardingSlaveBase::mimetype(url);
}


void Nepomuk2::TimelineProtocol::stat(const KUrl& url)
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
        } else {
            ForwardingSlaveBase::stat(url);
        }
        break;

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


// only used for the queries
bool Nepomuk2::TimelineProtocol::rewriteUrl(const KUrl& url, KUrl& newURL)
{
    if (parseTimelineUrl(url, &m_date, &m_filename) == DayFolder) {
        newURL = buildTimelineQuery(m_date).toSearchUrl();
        newURL.addPath(m_filename);
        kDebug() << url << newURL;
        return true;
    } else {
        return false;
    }
}


void Nepomuk2::TimelineProtocol::prepareUDSEntry(KIO::UDSEntry& entry,
        bool listing) const
{
    // Do nothing
    // We do not want the default implemention which tries to change the UDS_NAME and some
    // other stuff
}


void Nepomuk2::TimelineProtocol::listDays(int month, int year)
{
    kDebug() << month << year;
    const int days = KGlobal::locale()->calendar()->daysInMonth(QDate(year, month, 1));
    for (int day = 1; day <= days; ++day) {
        QDate date(year, month, day);
        if (date <= QDate::currentDate() &&
                filesInDateRange(date)) {
            listEntry(createDayUDSEntry(date), false);
        }
    }
}


void Nepomuk2::TimelineProtocol::listThisYearsMonths()
{
    kDebug();
    int currentMonth = QDate::currentDate().month();
    for (int month = 1; month <= currentMonth; ++month) {
        const QDate dateInMonth(QDate::currentDate().year(), month, 1);
        if (filesInDateRange(KGlobal::locale()->calendar()->firstDayOfMonth(dateInMonth),
                             KGlobal::locale()->calendar()->lastDayOfMonth(dateInMonth))) {
            listEntry(createMonthUDSEntry(month, QDate::currentDate().year()), false);
        }
    }
}


void Nepomuk2::TimelineProtocol::listPreviousYears()
{
    kDebug();
    // TODO: list years before this year that have files, but first get the smallest date
    // Using a query like: "select ?date where { ?r a nfo:FileDataObject . ?r nie:lastModified ?date . } ORDER BY ?date LIMIT 1" (this would have to be cached)
}


extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // necessary to use other kio slaves
        KComponentData("kio_timeline");
        QCoreApplication app(argc, argv);

        kDebug(7102) << "Starting timeline slave " << getpid();

        if (argc != 4) {
            kError() << "Usage: kio_timeline protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        Nepomuk2::TimelineProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug(7102) << "Timeline slave Done";

        return 0;
    }
}

#include "kio_timeline.moc"
