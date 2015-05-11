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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>

#include <signal.h>
#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QLatin1String("url"), QString());
    parser.addHelpOption();

    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() == 0) {
        QTextStream err(stderr);
        err << "Must input url/id of the file to be indexed";

        return 1;
    }

    QByteArray failArr = qgetenv("BALOO_EXTRACTOR_FAIL_FILE");
    QByteArray timeoutArr = qgetenv("BALOO_EXTRACTOR_TIMEOUT_FILE");
    if (failArr.isEmpty() && timeoutArr.isEmpty()) {
        return 0;
    }

    QStringList failFiles = QString::fromUtf8(failArr).split(QLatin1String(","), QString::SkipEmptyParts);
    QStringList timeoutFiles = QString::fromUtf8(timeoutArr).split(QLatin1String(","), QString::SkipEmptyParts);

    Q_FOREACH (const QString& fid, args) {
        if (failFiles.contains(fid)) {
            // kill oneself
            raise(SIGKILL);
            return -1;
        }

        if (timeoutFiles.contains(fid)) {
            // 100 msecs
            usleep(500 * 1000);
        }
    }

    return 0;
}
