/*
   Copyright (c) 2013 Vishesh Handa <me@vhanda.in>

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

#include <QCoreApplication>
#include <QUrl>

#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KComponentData>
#include <KDebug>

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
    KAboutData aboutData("baloosearch",
                         "baloosearch",
                         ki18n("Baloo Search"),
                         "0.1",
                         ki18n("Baloo Search - A debugging tool"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2013, Vishesh Handa"),
                         KLocalizedString(),
                         "http://kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+query", ki18n("The words to search for"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0) {
        KCmdLineArgs::usage();
        return 1;
    }

    QCoreApplication app(argc, argv);
    KComponentData comp(aboutData);

    QTextStream out(stdout);

    // HACK: Find a better way!
    QStringList argList = args->allArguments();
    argList.takeFirst();
    QString queryStr = argList.join(" ");

    Baloo::Query query;
    query.addType("File");
    query.setSearchString(queryStr);
    query.setLimit(10);

    out << "\n";
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QUrl url = iter.url();

        QString title;
        if (url.isLocalFile()) {
            title = colorString(url.toLocalFile(), 32);
        }
        /*
        else {
            QUrl resUri = result.resource().uri();
            title = colorString(url.toString(), 32) + "  " +
                    colorString(fetchProperty(resUri, NMO::sentDate()), 31) + " " +
                    colorString(fetchProperty(resUri, NAO::prefLabel()), 32);
        }
        */

        out << "  " << title << endl;
        //out << "  " << highlightBold( result.excerpt() ) << endl;
        out << endl;
    }

    return 0;
}
