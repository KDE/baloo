/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013-2015 Vishesh Handa <vhanda@kde.org>
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
#include <QFileInfo>
#include <QTextStream>
#include <QElapsedTimer>

#include <KAboutData>
#include <KLocalizedString>

#include "query.h"
#include "searchstore.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData(QStringLiteral("Baloo"),
                         i18n("Baloo Search"),
                         PROJECT_VERSION,
                         i18n("A tool to search through the files indexed by Baloo"),
                         KAboutLicense::GPL);
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QStringLiteral("vhanda@kde.org"));

    QCoreApplication app(argc, argv);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("l") << QStringLiteral("limit"),
                                        i18n("The maximum number of results"),
                                        i18n("limit")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("offset"),
                                        i18n("Offset from which to start the search"),
                                        i18n("offset")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("t") << QStringLiteral("type"),
                                        i18n("Type of data to be searched"),
                                        i18n("typeStr")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("d") << QStringLiteral("directory"),
                                        i18n("Limit search to specified directory"),
                                        i18n("directory")));
    parser.addPositionalArgument(i18n("query"), i18n("List of words to query for"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    int queryLimit = -1;
    int offset = 0;
    QString typeStr;

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    if (parser.isSet(QStringLiteral("type")))
        typeStr = parser.value(QStringLiteral("type"));
    if (parser.isSet(QStringLiteral("limit")))
        queryLimit = parser.value(QStringLiteral("limit")).toInt();
    if (parser.isSet(QStringLiteral("offset")))
        offset = parser.value(QStringLiteral("offset")).toInt();

    QTextStream out(stdout);

    QString queryStr = args.join(QStringLiteral(" "));

    Baloo::Query query;
    query.addType(typeStr);
    query.setSearchString(queryStr);
    query.setLimit(queryLimit);
    query.setOffset(offset);

    if (parser.isSet(QStringLiteral("directory"))) {
        QString folderName = parser.value(QStringLiteral("directory"));
        query.setIncludeFolder(QFileInfo(folderName).canonicalFilePath());
    }

    QElapsedTimer timer;
    timer.start();

    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QString filePath = iter.filePath();
        out << filePath << endl;
    }
    out << "Elapsed: " << timer.nsecsElapsed() / 1000000.0 << " msecs" << endl;

    return 0;
}
