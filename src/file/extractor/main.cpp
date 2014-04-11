/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-13 Vishesh Handa <handa.vish@gmail.com>
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

#include <k4aboutdata.h>
#include <KCmdLineArgs>
#include <KLocale>
#include <KComponentData>
#include <QApplication>

#include <QDebug>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    K4AboutData aboutData("baloo_file_extractor", 0, ki18n("Baloo File Extractor"),
                         "0.1",
                         ki18n("The File Extractor extracts the file metadata and text"),
                         K4AboutData::License_LGPL_V2,
                         ki18n("(C) 2013, Vishesh Handa"));
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[url]", ki18n("The URL/id of the files to be indexed"));
    options.add("debug", ki18n("Print the data being indexed"));
    options.add("bdata", ki18n("Print the QVariantMap in Base64 encoding"));
    // FIXME: Set a proper string after the freeze. This option is just for debugging
    options.add("db <url>", KLocalizedString());

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    int argCount = args->count();
    if (argCount == 0) {
        QTextStream err(stderr);
        err << "Must input url/id of the file to be indexed";

        return 1;
    }

    if (args->isSet("bdata") && argCount > 1) {
        QTextStream err(stderr);
        err << "bdata can only accept one url/id";

        return 1;
    }

    QApplication app(argc, argv);
    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    Baloo::App appObject;
    return app.exec();
}
