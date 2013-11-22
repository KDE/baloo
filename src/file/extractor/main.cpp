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
#include "../database.h"
#include "filemapping.h"
#include "../util.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KComponentData>
#include <KApplication>
#include <KStandardDirs>

#include <QApplication>
#include <qjson/serializer.h>

#include <iostream>
#include <KDebug>
#include <KUrl>
#include <KMimeType>

#include <kfilemetadata/extractorpluginmanager.h>
#include <kfilemetadata/extractorplugin.h>

#include <xapian.h>

using namespace Baloo;

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
    options.add("+[url]", ki18n("The URL of the files to be indexed"));
    options.add("debug", ki18n("Print the data being indexed"));

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    // Application
    QApplication app(argc, argv);
    KComponentData data(aboutData, KComponentData::RegisterAsMainComponent);

    int argCount = args->count();
    if (argCount == 0) {
        QTextStream err(stderr);
        err << "Must input url of the file to be indexed";

        return 1;
    }

    // FIXME: What if the fileMapping does not exist?
    // FIXME: We do not need the readonly db
    QString path = KStandardDirs::locateLocal("data", "baloo/file");

    Database bigDb;
    bigDb.setPath(path);
    bigDb.init();
    QSqlDatabase& sqlDb = bigDb.sqlDatabase();

    Xapian::WritableDatabase db;
    try {
        db = Xapian::WritableDatabase(path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    }
    catch (const Xapian::DatabaseLockError& err) {
        kError() << err.get_error_string();
        return 1;
    }

    KFileMetaData::ExtractorPluginManager m_manager;
    for (int i=0; i<argCount; i++) {
        const QString url = args->url(i).toLocalFile();
        const QString mimetype = KMimeType::findByUrl(args->url(i))->name();

        QList<KFileMetaData::ExtractorPlugin*> exList = m_manager.fetchExtractors(mimetype);

        QVariantMap map;
        Q_FOREACH (KFileMetaData::ExtractorPlugin* plugin, exList) {
            QVariantMap data = plugin->extract(url, mimetype);
            map.unite(data);
        }

        FileMapping file(url);
        if (!file.fetch(sqlDb))
            continue;

        Xapian::Document doc = db.get_document(file.id());
        Xapian::TermGenerator termGen;
        termGen.set_document(doc);
        termGen.set_database(db);

        QJson::Serializer serializer;
        QByteArray json = serializer.serialize(map);

        doc.set_data(json.constData());

        QMap<QString, QVariant>::const_iterator it = map.constBegin();
        for (; it != map.constEnd(); it++) {
            const QString key = it.key().toUpper();
            const QVariant val = it.value();

            if (val.type() == QVariant::Bool) {
                doc.add_boolean_term(key.toStdString());
            }
            else if (val.type() == QVariant::Int) {
                const QString term = key + val.toString();
                doc.add_term(term.toStdString());
            }
            else if (val.type() == QVariant::DateTime) {
                const QString term = key + val.toDateTime().toString(Qt::ISODate);
                doc.add_term(term.toStdString());
            }
            else {
                QString term = key;
                if (term == "TEXT")
                    term.clear();

                if (term.size())
                    termGen.index_text(val.toString().toStdString(), 1, term.toStdString());
                termGen.index_text(val.toString().toStdString());
            }
        }

        db.replace_document(file.id(), doc);
        updateIndexingLevel(db, file.id(), 2);

        if (args->isSet("debug"))
            kDebug() << map;
    }

    db.commit();

    return 0;
}
