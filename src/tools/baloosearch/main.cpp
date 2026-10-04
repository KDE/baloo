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
#include <KFileMetaData/TypeInfo>
#include <KLocalizedString>

#include "query.h"

#include <iostream>

using namespace Qt::StringLiterals;

void showKFMAllTypes(int exitCode)
{
    const auto allNames = KFileMetaData::TypeInfo::allNames();

    QStringList output;
    output.reserve(3 + allNames.size());
    const auto header = i18nc("'listTypes' table header", "<typeString>     Description");
    output.append(header);
    output.append(QString(header.size(), u'='));

    for (const auto &name : allNames) {
        const auto &typeInfo = KFileMetaData::TypeInfo::fromName(name);
        output.append(u"%1 %2"_s.arg(name, -16).arg(typeInfo.displayName()));
    }
    output.append(u""_s);

    QCommandLineParser::showMessageAndExit( //
        exitCode == 0 ? QCommandLineParser::MessageType::Information : QCommandLineParser::MessageType::Error,
        output.join(u'\n'),
        exitCode);
}

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
    parser.addOption(QCommandLineOption({u"l"_s, u"limit"_s}, //
                                        i18n("The maximum number of results"),
                                        i18nc("option value name", "limit")));
    parser.addOption(QCommandLineOption({u"o"_s, u"offset"_s}, //
                                        i18n("Offset from which to start the search"),
                                        i18nc("option value name", "offset")));
    parser.addOption(QCommandLineOption({u"t"_s, u"type"_s}, //
                                        i18n("Type of data to be searched"),
                                        i18nc("option value name", "typeString")));
    parser.addOption(QCommandLineOption(u"listTypes"_s, //
                                        i18n("Show supported <typeString> values")));
    parser.addOption(QCommandLineOption({u"d"_s, u"directory"_s}, //
                                        i18n("Limit search to specified directory"),
                                        i18nc("option value name", "directory")));
    parser.addOption(QCommandLineOption({u"i"_s, u"id"_s}, //
                                        i18n("Show document IDs")));
    parser.addOption(QCommandLineOption({u"s"_s, u"sort"_s}, //
                                        i18n("Sorting criteria"),
                                        u"auto|time|none"_s,
                                        u"auto"_s));
    parser.addPositionalArgument(i18n("query"), i18n("List of words to query for"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    int queryLimit = -1;
    int offset = 0;
    QString typeStr;
    bool showDocumentId = parser.isSet(u"id"_s);

    if (parser.isSet(u"listTypes"_s)) {
        showKFMAllTypes(0);
    }

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    if (parser.isSet(u"type"_s)) {
        typeStr = parser.value(u"type"_s);
        const auto typeinfo = KFileMetaData::TypeInfo::fromName(typeStr);
        if (typeinfo.type() == KFileMetaData::Type::Empty) {
            std::cerr << qPrintable(i18n("ERROR: Invalid \"type\" value, supported values:\n")) << std::endl;
            showKFMAllTypes(1);
        }
    }
    if (parser.isSet(u"limit"_s)) {
        queryLimit = parser.value(u"limit"_s).toInt();
    }
    if (parser.isSet(u"offset"_s)) {
        offset = parser.value(u"offset"_s).toInt();
    }
    const Baloo::Query::SortingOption orderBy = [&parser]() {
        auto val = parser.value(u"sort"_s);
        if (val == u"auto"_s) {
            return Baloo::Query::SortAuto;
        } else if (val == u"time"_s) {
            return Baloo::Query::SortAuto;
        } else if (val == u"none"_s) {
            return Baloo::Query::SortNone;
        } else {
            parser.showHelp(1);
        }
    }();

    QString queryStr = args.join(QLatin1Char(' '));

    Baloo::Query query;
    query.addType(typeStr);
    query.setSearchString(queryStr);
    query.setLimit(queryLimit);
    query.setOffset(offset);
    query.setSortingOption(orderBy);

    if (parser.isSet(u"directory"_s)) {
        QString folderName = parser.value(u"directory"_s);
        const QFileInfo fi(folderName);
        if (!fi.isDir()) {
            std::cerr << qPrintable(i18n("%1 is not a valid directory", folderName)) << std::endl;
            return 1;
        }
        while (folderName.endsWith(QLatin1Char('/')) && (folderName.size() > 1)) {
            folderName.chop(1);
        }
        auto canonicalPath = fi.canonicalFilePath();
        if (canonicalPath != folderName) {
            std::cerr << qPrintable(i18n("Using canonical path '%1' for '%2'", canonicalPath, folderName)) << std::endl;
        }
        query.setIncludeFolder(canonicalPath);
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
