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
#include <QDir>
#include <QTime>
#include <iostream>

#include "kinotify.h"
#include "util.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    KInotify inotify(nullptr /*no config*/);
    QObject::connect(&inotify, &KInotify::installedWatches,
                     &app, &QCoreApplication::quit);

    QObject::connect(&inotify, &KInotify::attributeChanged,
                     [](const QString& fileUrl) { qDebug() << "AttrbuteChanged:" << fileUrl; });
    QObject::connect(&inotify, &KInotify::created,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Created:" << fileUrl << isDir; });
    QObject::connect(&inotify, &KInotify::deleted,
                     [](const QString& fileUrl, bool isDir) { qDebug() << "Deleted:" << fileUrl << isDir; });
    QObject::connect(&inotify, &KInotify::modified,
                     [](const QString& fileUrl) { qDebug() << "Modified:" << fileUrl; });
    QObject::connect(&inotify, &KInotify::closedWrite,
                     [](const QString& fileUrl) { qDebug() << "ClosedWrite:" << fileUrl; });

    QTime timer;
    timer.start();

    KInotify::WatchEvents flags(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf
                                | KInotify::EventCloseWrite | KInotify::EventCreate
                                | KInotify::EventAttributeChange | KInotify::EventModify);
    inotify.addWatch(QDir::homePath(), flags);
    app.exec();

    std::cout << "Elapsed: " << timer.elapsed() << std::endl;
    printIOUsage();

    return app.exec();
}
