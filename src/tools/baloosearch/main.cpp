/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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

#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("Baloo"),
                         i18n("Baloo Search"),
                         QStringLiteral(PROJECT_VERSION),
                         i18n("A tool to search through the files indexed by Baloo"),
                         KAboutLicense::GPL);
    aboutData.addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));

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
    parser.addOption(QCommandLineOption({QStringLiteral("i"), QStringLiteral("id")},
                                        i18n("Show document IDs")));
    parser.addPositionalArgument(i18n("query"), i18n("List of words to query for"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    int queryLimit = -1;
    int offset = 0;
    QString typeStr;
    bool showDocumentId = parser.isSet(QStringLiteral("id"));

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    if (parser.isSet(QStringLiteral("type"))) {
        typeStr = parser.value(QStringLiteral("type"));
    }
    if (parser.isSet(QStringLiteral("limit"))) {
        queryLimit = parser.value(QStringLiteral("limit")).toInt();
    }
    if (parser.isSet(QStringLiteral("offset"))) {
        offset = parser.value(QStringLiteral("offset")).toInt();
    }

    QString queryStr = args.join(QLatin1Char(' '));

    Baloo::Query query;
    query.addType(typeStr);
    query.setSearchString(queryStr);
    query.setLimit(queryLimit);
    query.setOffset(offset);

    if (parser.isSet(QStringLiteral("directory"))) {
        QString folderName = parser.value(QStringLiteral("directory"));
        const QFileInfo fi(folderName);
        if (!fi.isDir()) {
            std::cerr << qPrintable(i18n("%1 is not a valid directory", folderName)) << std::endl;
            return 1;
        }
        query.setIncludeFolder(fi.canonicalFilePath());
    }

    QElapsedTimer timer;
    timer.start();

    Baloo::ResultIterator iter = query.exec();
    while (iter.next()) {
        const QString filePath = iter.filePath();
        if (showDocumentId) {
            std::cout << iter.documentId().constData() << " ";
        }
        std::cout << qPrintable(filePath) << std::endl;
    }
    std::cerr << qPrintable(i18n("Elapsed: %1 msecs", timer.nsecsElapsed() / 1000000.0)) << std::endl;

    return 0;
}
