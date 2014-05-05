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
#include <QFile>

#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KComponentData>
#include <KDebug>
#include <KUrl>
#include <KConfigGroup>
#include <KStandardDirs>
#include <QProcess>

#include <QDBusMessage>
#include <QDBusConnection>

int main(int argc, char* argv[])
{
    KAboutData aboutData("balooctl", "balooctl", KLocalizedString(), "0.1");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n("Maintainer"), "me@vhanda.in");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    //options.add("status", ki18n("Print the status of the indexer"));
    options.add("+enable", ki18n("Enable the file indexer"));
    options.add("+disable", ki18n("Disable the file indexer"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QCoreApplication app(argc, argv);
    KComponentData comp(aboutData);

    if (args->count() == 0)
        KCmdLineArgs::usage();

    QTextStream err(stderr);
    QTextStream out(stdout);

    QString command = args->arg(0);
    if (command == QLatin1String("status")) {
        // Get info if baloo_file is running
        // Get number of level 1 files
        // Get number of level 2 files
        // Print a kind of progress bar
        return 0;
    }

    bool isEnabled = false;
    if (command == QLatin1String("enable")) {
        isEnabled = true;
    }
    else if (command == QLatin1String("disable")) {
        isEnabled = false;
    }
    else {
        return 1;
    }

    KConfig config("baloofilerc");
    KConfigGroup basicSettings = config.group("Basic Settings");
    basicSettings.writeEntry("Indexing-Enabled", isEnabled);

    if (isEnabled) {
        out << "Enabling the File Indexer\n";
        config.group("General").writeEntry("first run", true);

        const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file"));
        QProcess::startDetached(exe);
    }
    else {
        out << "Disabling the File Indexer\n";

        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.kde.baloo.file"),
                                                              QLatin1String("/indexer"),
                                                              QLatin1String("org.kde.baloo.file"),
                                                              QLatin1String("quit"));
        QDBusConnection::sessionBus().call(message);

        const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_cleaner"));
        QProcess::startDetached(exe);
    }

    return 0;
}
