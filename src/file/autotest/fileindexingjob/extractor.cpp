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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KComponentData>

#include <QApplication>
#include <QTextStream>
#include <QDebug>

#include <signal.h>
#include <iostream>

int main(int argc, char* argv[])
{
    KAboutData aboutData("baloo_file_extractor_dummy", 0, KLocalizedString(),
                         "0.1", KLocalizedString());

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[url]", KLocalizedString());

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    int argCount = args->count();
    if (argCount == 0) {
        QTextStream err(stderr);
        err << "Must input url/id of the file to be indexed";

        return 1;
    }

    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    QByteArray arr = qgetenv("BALOO_EXTRACTOR_FAIL_FILE");
    if (arr.isEmpty()) {
        return 0;
    }

    QStringList failFiles = QString::fromUtf8(arr).split(",", QString::SkipEmptyParts);

    for (int i = 0; i < args->count(); i++) {
        QString fid = args->arg(i);
        if (failFiles.contains(fid)) {
            // kill oneself
            raise(SIGKILL);
            return -1;
        }
    }

    return 0;
}
