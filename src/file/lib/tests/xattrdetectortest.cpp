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

#include "xattrdetector.h"

#include <k4aboutdata.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <QDebug>

#include <QUrl>
#include <QTimer>

#include <iostream>

int main(int argc, char** argv)
{
    K4AboutData aboutData("xattrdetectortest",
                         "xattrdetectortest",
                         ki18n("xattrdetectortest"),
                         "0.1",
                         ki18n("xattrdetectortest"),
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

    Baloo::XattrDetector xattr;

    const QUrl url = args->url(0);
    bool shouldHave = xattr.isSupported(url.toLocalFile());
    std::cout << url.toLocalFile().toUtf8().constData() << " " << shouldHave << std::endl;

    return 0;
}

