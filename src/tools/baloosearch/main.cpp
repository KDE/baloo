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

#include <QCoreApplication>
#include <QUrl>

#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KComponentData>
#include <KDebug>
#include <k4aboutdata.h>

#include "query.h"

QString highlightBold(const QString& input)
{
    QLatin1String colorStart("\033[0;33m");
    QLatin1String colorEnd("\033[0;0m");

    QString out(input);
    out.replace("<b>", colorStart);
    out.replace("</b>", colorEnd);

    return out;
}

QString colorString(const QString& input, int color)
{
    QString colorStart = QString::fromLatin1("\033[0;%1m").arg(color);
    QLatin1String colorEnd("\033[0;0m");

    return colorStart + input + colorEnd;
}

int main(int argc, char* argv[])
{
    K4AboutData aboutData("baloosearch",
                         "baloosearch",
                         ki18n("Baloo Search"),
                         "0.1",
                         ki18n("Baloo Search - A debugging tool"),
                         K4AboutData::License_GPL,
                         ki18n("(c) 2013, Vishesh Handa"),
                         KLocalizedString(),
                         "http://kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("l").add("limit <queryLimit>",  ki18n("Number of results to return"));
    options.add("o").add("offset <offset>",  ki18n("Offset from which start the search"));
    options.add("t").add("type <typeStr>",  ki18n("Type of data to be searched"));
    options.add("e").add("email",  ki18n("If set, search through emails"));
    options.add("+query", ki18n("The words to search for"));
    KCmdLineArgs::addCmdLineOptions(options);

    int queryLimit = 10;
    int offset = 0;
    QString typeStr = "File";

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0) {
        KCmdLineArgs::usage();
        return 1;
    }

    if(args->isSet("type"))
        typeStr = args->getOption("type");
    if(args->isSet("limit"))
        queryLimit = args->getOption("limit").toInt();
    if(args->isSet("offset"))
        offset = args->getOption("offset").toInt();

    if(args->isSet("email"))
        typeStr = "Email";

    QCoreApplication app(argc, argv);
    KComponentData comp(aboutData);

    QTextStream out(stdout);

    QString queryStr = args->arg(0);
    for (int i = 1; i < args->count(); i++) {
        queryStr += " ";
        queryStr += args->arg(i);
    }

    Baloo::Query query;
    query.addType(typeStr);
    query.setSearchString(queryStr);
    query.setLimit(queryLimit);
    query.setOffset(offset);

    out << "\n";
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QUrl url = iter.url();

        QString title;
        if (url.isLocalFile()) {
            title = colorString(url.toLocalFile(), 32);
        }
        else {
            title = colorString(QString::fromUtf8(iter.id()), 31);
            title += " ";
            title += colorString(iter.text(), 32);
        }

        out << "  " << title << endl;
        //out << "  " << highlightBold( result.excerpt() ) << endl;
        out << endl;
    }

    return 0;
}
