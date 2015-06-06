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

#include <KLocalizedString>
#include <QStandardPaths>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    setIdleSchedulingPriority();
    lowerPriority();

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("Baloo File Extractor"));
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("The File Extractor extracts the file metadata and text"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo");
    Baloo::App appObject(path);
    return app.exec();
}
