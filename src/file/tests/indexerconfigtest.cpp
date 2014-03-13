/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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

#include "../fileindexerconfig.h"

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <k4aboutdata.h>
#include <kmimetype.h>

#include <QTimer>

#include <KUrl>
#include <iostream>

int main(int argc, char** argv)
{
    K4AboutData aboutData("indexerconfigtest",
                         "indexerconfigtest",
                         ki18n("indexerconfigtest"),
                         "0.1",
                         ki18n("indexerconfigtest"),
                         K4AboutData::License_GPL,
                         ki18n("(c) 2014, Vishesh Handa"),
                         KLocalizedString(),
                         "http://kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+file", ki18n("The file URL"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QCoreApplication app(argc, argv);
    KComponentData comp(aboutData);

    if (args->count() == 0)
        KCmdLineArgs::usage();

    Baloo::FileIndexerConfig config;

    const QUrl url = args->url(0);
    bool shouldIndex = config.shouldBeIndexed(url.toLocalFile());

    QString mimetype = KMimeType::findByUrl(url)->name();
    bool shouldIndexMimetype = config.shouldMimeTypeBeIndexed(mimetype);
    std::cout << url.toLocalFile().toUtf8().constData() << "\n"
              << "Should Index: " << std::boolalpha << shouldIndex << "\n"
              << "Should Index Mimetype: " << std::boolalpha << shouldIndexMimetype << "\n"
              << "Mimetype: " << mimetype.toUtf8().constData() << std::endl;

    return 0; //app.exec();
}
