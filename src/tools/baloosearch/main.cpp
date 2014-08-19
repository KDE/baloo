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
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUrl>
#include <QDebug>

#include <KAboutData>
#include <KLocalizedString>

#include <iostream>
#include "query.h"

QString highlightBold(const QString& input)
{
    QLatin1String colorStart("\033[0;33m");
    QLatin1String colorEnd("\033[0;0m");

    QString out(input);
    out.replace(QLatin1String("<b>"), colorStart);
    out.replace(QLatin1String("</b>"), colorEnd);

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
    KAboutData aboutData(QLatin1String("baloosearch"),
                         i18n("Baloo Search"),
                         PROJECT_VERSION,
                         i18n("Baloo Search - A debugging tool"),
                         KAboutLicense::GPL,
                         i18n("(c) 2013, Vishesh Handa"));
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QLatin1String("me@vhanda.in"));

    KAboutData::setApplicationData(aboutData);
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("l") << QLatin1String("limit"),
                                        QLatin1String("The maximum number of results"),
                                        QLatin1String("limit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("o") << QLatin1String("offset"),
                                        QLatin1String("Offset from which to start the search"),
                                        QLatin1String("offset")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("t") << QLatin1String("type"),
                                        QLatin1String("Type of data to be searched"),
                                        QLatin1String("typeStr")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("e") << QLatin1String("email"),
                                        QLatin1String("If set, search for email")));
    parser.addPositionalArgument(QLatin1String("query"), QLatin1String("List of words to query for"));
    parser.process(app);

    int queryLimit = 10;
    int offset = 0;
    QString typeStr = QLatin1String("File");

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    if(parser.isSet(QLatin1String("type")))
        typeStr = parser.value(QLatin1String("type"));
    if(parser.isSet(QLatin1String("limit")))
        queryLimit = parser.value(QLatin1String("limit")).toInt();
    if(parser.isSet(QLatin1String("offset")))
        offset = parser.value(QLatin1String("offset")).toInt();

    if(parser.isSet(QLatin1String("email")))
        typeStr = QLatin1String("Email");

    QTextStream out(stdout);

    QString queryStr = args.join(QLatin1String(" "));

    Baloo::Query query;
    query.addType(typeStr);
    query.setSearchString(queryStr);
    query.setLimit(queryLimit);
    query.setOffset(offset);

    out << "\n";
    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QUrl url = iter.url();
        const QByteArray id = iter.id();
        int fid = Baloo::deserialize("file", id);

        QString title;
        if (url.isLocalFile()) {
            title = colorString(QString::number(fid), 31) + QLatin1String(" ") + colorString(url.toLocalFile(), 32);
        }
        else {
            title = colorString(QString::fromUtf8(iter.id()), 31);
            title += QLatin1String(" ");
            title += colorString(iter.text(), 32);
        }

        out << "  " << title << endl;
        //out << "  " << highlightBold( result.excerpt() ) << endl;
        out << endl;
    }

    return 0;
}
