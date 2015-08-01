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

#include <KAboutData>
#include <KLocalizedString>
#include <QStandardPaths>
#include <QByteArray>
#include <QDBusConnection>

#include <QCoreApplication>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    setIdleSchedulingPriority();
    lowerPriority();

    KAboutData aboutData(QStringLiteral("baloo"), i18n("Baloo File Extractor"), PROJECT_VERSION);
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QStringLiteral("vhanda@kde.org"), QStringLiteral("http://vhanda.in"));

    KAboutData::setApplicationData(aboutData);

    QCoreApplication app(argc, argv);

    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.baloo.extractor"));

    QString path = qgetenv("BALOO_DB_PATH");
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo");
    }

    Baloo::App appObject(path);
    return app.exec();
}
