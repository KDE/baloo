/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-14 Vishesh Handa <handa.vish@gmail.com>
   Copyright (C) 2010-2011 Sebastian Trueg <trueg@kde.org>

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

#include "app.h"
#include "../priority.h"

#include <KLocale>
#include <KGlobal>
#include <KStandardDirs>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Baloo File Extractor");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("The File Extractor extracts the file metadata and text"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("urls", i18n("The URL/id of the files to be indexed"));
    parser.addOption(QCommandLineOption("debug", i18n("Print the data being indexed")));
    parser.addOption(QCommandLineOption("bdata", i18n("Print the QVariantMap in Base64 encoding")));
    parser.addOption(QCommandLineOption("db", i18n("Specify a custom path for the database"),
                                        i18n("path"), KGlobal::dirs()->localxdgdatadir() + "baloo/file"));

    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        fprintf(stderr, "The url/id of the file is missing\n\n");
        parser.showHelp(1);
    }

    if (parser.isSet("bdata") && args.size() > 1) {
        fprintf(stderr, "bdata can only accept one url/id\n\n");
        parser.showHelp(1);
    }

    Baloo::App appObject(parser.value("db"));
    appObject.setBData(parser.isSet("bdata"));
    appObject.setDebug(parser.isSet("debug"));
    appObject.startProcessing(args);

    return app.exec();
}
