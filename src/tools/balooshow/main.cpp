/*
   Copyright (c) 2012-2013 Vishesh Handa <me@vhanda.in>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QHash>
#include <QUrl>
#include <QFile>

#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KComponentData>
#include <QDebug>
#include <k4aboutdata.h>

#include "file.h"
#include "filefetchjob.h"
#include "searchstore.h" // for deserialize

#include <kfilemetadata/propertyinfo.h>

QString colorString(const QString& input, int color)
{
    QString colorStart = QString::fromLatin1("\033[0;%1m").arg(color);
    QLatin1String colorEnd("\033[0;0m");

    return colorStart + input + colorEnd;
}

int main(int argc, char* argv[])
{
    K4AboutData aboutData("balooshow",
                         "balooshow",
                         ki18n("Baloo Show"),
                         "0.1",
                         ki18n("The Baloo data Viewer - A debugging tool"),
                         K4AboutData::License_GPL,
                         ki18n("(c) 2012, Vishesh Handa"),
                         KLocalizedString(),
                         "http://kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+resource", ki18n("The file URL"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QCoreApplication app(argc, argv);
    KComponentData comp(aboutData);

    if (args->count() == 0)
        KCmdLineArgs::usage();

    QTextStream err(stdout);

    //
    // The Resource Uri
    //
    QVector<QString> urls;
    for (int i = 0; i < args->count(); i++) {
        const QString url = args->url(i).toLocalFile();
        if (QFile::exists(url)) {
            urls.append(url);
        }
        else {
            urls.append(QLatin1String("file:") + args->arg(i));
        }
    }

    QTextStream stream(stdout);
    Baloo::FileFetchJob* job;
    QString text;

    Q_FOREACH (const QString& url, urls) {
        Baloo::File ifile;
        if (url.startsWith("file:")) {
            ifile.setId(url.toUtf8());
        }
        else {
            ifile.setUrl(url);
        }
        job = new Baloo::FileFetchJob(ifile);
        job->exec();

        Baloo::File file = job->file();
        int fid = Baloo::deserialize("file", file.id());

        if (fid && !file.url().isEmpty()) {
            text = colorString(QString::number(fid), 31);
            text += " ";
            text += colorString(file.url(), 32);
            stream << text << endl;
        }
        else {
            stream << "No index information found" << endl;
        }

        KFileMetaData::PropertyMap propMap = file.properties();
        KFileMetaData::PropertyMap::const_iterator it = propMap.constBegin();
        for (; it != propMap.constEnd(); ++it) {
            KFileMetaData::PropertyInfo pi(it.key());
            stream << "\t" << pi.displayName() << ": " << it.value().toString() << endl;
        }

        if (file.rating())
            stream << "\t" << "Rating: " << file.rating() << endl;

        if (!file.tags().isEmpty())
            stream << "\t" << "Tags: " << file.tags().join(", ") << endl;

        if (!file.userComment().isEmpty())
            stream << "\t" << "User Comment: " << file.userComment() << endl;
    }

    return 0;
}
