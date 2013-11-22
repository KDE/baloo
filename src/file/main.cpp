/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QApplication>

#include <KComponentData>
#include <KAboutData>
#include <KStandardDirs>

#include "filewatch.h"
#include "fileindexer.h"
#include "database.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    KAboutData aboutData("biloo_file", "biloo_file", ki18n("Biloo File"), "0.1",
                         ki18n("An application to handle file metadata"),
                         KAboutData::License_GPL_V2);

    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    Database db;
    db.setPath(KStandardDirs::locateLocal("data", "baloo/file/"));
    db.init();
    db.sqlDatabase().transaction();

    Baloo::FileWatch filewatcher(&db, &app);
    Baloo::FileIndexer fileIndexer(&db, &app);

    QObject::connect(&filewatcher, SIGNAL(indexFile(QString)),
                     &fileIndexer, SLOT(indexFile(QString)));
    return app.exec();
}
