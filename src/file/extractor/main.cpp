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

#include "priority.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KComponentData>
#include <KApplication>

#include <QApplication>
#include <QtCore/QDir>
#include <QtCore/QTextStream>

#include <iostream>
#include <KDebug>
#include <KUrl>
#include <KJob>
#include <KMimeType>

#include <kfilemetadata/extractorpluginmanager.h>
#include <kfilemetadata/extractorplugin.h>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    KAboutData aboutData("baloo_file_extractor", 0, ki18n("Baloo File Extractor"),
                         "0.1",
                         ki18n("The File Extractor extracts the file metadata and text"),
                         KAboutData::License_LGPL_V2,
                         ki18n("(C) 2013, Vishesh Handa"));
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[url]", ki18n("The URL of the file to be indexed"));

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // Application
    QApplication app(argc, argv);
    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    if (args->count() == 0) {
        QTextStream err(stderr);
        err << "Must input url of the file to be indexed";

        return 1;
    }

    const QString url = args->url(0).toLocalFile();
    QString mimetype = KMimeType::findByUrl(args->url(0))->name();

    KFileMetaData::ExtractorPluginManager m_manager;
    QList<KFileMetaData::ExtractorPlugin*> exList = m_manager.fetchExtractors(mimetype);

    QVariantMap map;
    Q_FOREACH (KFileMetaData::ExtractorPlugin* plugin, exList) {
        QVariantMap data = plugin->extract(url, mimetype);
        map.unite(data);
    }

    QByteArray byteArray;
    QDataStream st(&byteArray, QIODevice::WriteOnly);
    st << map;

    std::cout << byteArray.toBase64().constData();

    /*
    QMap<QString, QVariant>::const_iterator it = map.constBegin();
    for (; it != map.constEnd(); it++) {
        std::cout << it.key().toStdString() << ": " << it.value().toString().toStdString() << '\n';
    }*/

    return 0;
}
