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
                         i18n("Baloo Search"),
                         "0.1",
                         i18n("Baloo Search - A debugging tool"),
                         KAboutLicense::GPL,
                         i18n("(c) 2013, Vishesh Handa"));
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), "me@vhanda.in");

    KAboutData::setApplicationData(aboutData);
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << "l" << "limit",
                                        "The maximum number of results",
                                        "limit"));
    parser.addOption(QCommandLineOption(QStringList() << "o" << "offset",
                                        "Offset from which to start the search",
                                        "offset"));
    parser.addOption(QCommandLineOption(QStringList() << "t" << "type",
                                        "Type of data to be searched",
                                        "typeStr"));
    parser.addOption(QCommandLineOption(QStringList() << "e" << "email",
                                        "If set, search for email"));
    parser.addPositionalArgument("query", "List of words to query for");
    parser.process(app);

    int queryLimit = 10;
    int offset = 0;
    QString typeStr = "File";

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    if(parser.isSet("type"))
        typeStr = parser.value("type");
    if(parser.isSet("limit"))
        queryLimit = parser.value("limit").toInt();
    if(parser.isSet("offset"))
        offset = parser.value("offset").toInt();

    if(parser.isSet("email"))
        typeStr = "Email";

    QTextStream out(stdout);

    QString queryStr = args.join(" ");

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
            title = colorString(QString::number(fid), 31) + " " + colorString(url.toLocalFile(), 32);
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
