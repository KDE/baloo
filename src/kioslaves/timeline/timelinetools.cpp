/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "timelinetools.h"
#include "kio_timeline_debug.h"

#include <QUrl>
#include <QDate>
#include <QRegExp>
#include <QDebug>
#include <QUrlQuery>

#include <KLocalizedString>

namespace
{
QDate applyRelativeDateModificators(const QDate& date, const QMap<QString, QString>& modificators)
{
    QDate newDate(date);
    const QString dayKey = QStringLiteral("relDays");
    const QString weekKey = QStringLiteral("relWeeks");
    const QString monthKey = QStringLiteral("relMonths");
    const QString yearKey = QStringLiteral("relYears");
    bool ok = false;

    if (modificators.contains(yearKey)) {
        int relYears = modificators[yearKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addYears(relYears);
        }
    }
    if (modificators.contains(monthKey)) {
        int relMonths = modificators[monthKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addMonths(relMonths);
        }
    }
    if (modificators.contains(weekKey)) {
        int relWeeks = modificators[weekKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addDays(relWeeks * 7); // we assume weeks have 7 days everywhere. QDate seems to make that assumption too, should be OK.
        }
    }
    if (modificators.contains(dayKey)) {
        int relDays = modificators[dayKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addDays(relDays);
        }
    }
    return newDate;
}
}


Baloo::TimelineFolderType Baloo::parseTimelineUrl(const QUrl& url, QDate* date, QString* filename)
{
    qCDebug(KIO_TIMELINE) << url;

    static QRegExp s_dateRegexp(QStringLiteral("\\d{4}-\\d{2}(?:-(\\d{2}))?"));

    // reset
    *date = QDate();

    QString path = url.path();
    if (path.endsWith(QLatin1Char('/')))
        path = path.mid(0, path.length()-1);

    if (path.isEmpty() || path == QLatin1String("/")) {
        qCDebug(KIO_TIMELINE) << url << "is root folder";
        return RootFolder;
    } else if (path.startsWith(QLatin1String("/today"))) {
        *date = QDate::currentDate();
        if (filename)
            *filename = path.mid(7);
        qCDebug(KIO_TIMELINE) << url << "is today folder:" << *date;
        return DayFolder;
    } else if (path == QLatin1String("/calendar")) {
        qCDebug(KIO_TIMELINE) << url << "is calendar folder";
        return CalendarFolder;
    } else {
        QStringList sections = path.split(QStringLiteral("/"), QString::SkipEmptyParts);
        QString dateString;
        if (s_dateRegexp.exactMatch(sections.last())) {
            dateString = sections.last();
        } else if (sections.count() > 1 && s_dateRegexp.exactMatch(sections[sections.count() - 2])) {
            dateString = sections[sections.count() - 2];
            if (filename)
                *filename = sections.last();
        } else {
            qCWarning(KIO_TIMELINE) << url << "COULD NOT PARSE";
            return NoFolder;
        }

        if (s_dateRegexp.cap(1).isEmpty()) {
            // no day -> month listing
            qCDebug(KIO_TIMELINE) << "parsing " << dateString;
            *date = QDate::fromString(dateString, QStringLiteral("yyyy-MM"));
            qCDebug(KIO_TIMELINE) << url << "is month folder:" << date->month() << date->year();
            if (date->month() > 0 && date->year() > 0)
                return MonthFolder;
        } else {
            qCDebug(KIO_TIMELINE) << "parsing " << dateString;
            typedef QPair<QString, QString> StringPair;
            QUrlQuery query(url);
            QList<StringPair> queryItems = query.queryItems();
            QMap<QString, QString> map;
            Q_FOREACH (const StringPair& pair, queryItems) {
                map.insert(pair.first, pair.second);
            }

            *date = applyRelativeDateModificators(QDate::fromString(dateString, QStringLiteral("yyyy-MM-dd")), map);
            // only in day folders we can have filenames
            qCDebug(KIO_TIMELINE) << url << "is day folder:" << *date;
            if (date->isValid())
                return DayFolder;
        }
    }

    return NoFolder;
}
