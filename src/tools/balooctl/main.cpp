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
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QDebug>

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QStandardPaths>
#include <QProcess>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "xapiandatabase.h"
#include "filestatistics.h"

using namespace Baloo;

void start()
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file"));
    QProcess::startDetached(exe);
}

void stop()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.kde.baloo.file"),
                                                          QLatin1String("/indexer"),
                                                          QLatin1String("org.kde.baloo.file"),
                                                          QLatin1String("quit"));
    QDBusConnection::sessionBus().call(message);
}

int main(int argc, char* argv[])
{
    KAboutData aboutData(QLatin1String("balooctl"), i18n("balooctl"), QLatin1String("0.1"));
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QLatin1String("me@vhanda.in"));

    KAboutData::setApplicationData(aboutData);
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QLatin1String("command"), i18n("The command to execute"));

    parser.addPositionalArgument(QLatin1String("status"), i18n("Print the status of the indexer"));
    parser.addPositionalArgument(QLatin1String("enable"), i18n("Enable the file indexer"));
    parser.addPositionalArgument(QLatin1String("disable"), i18n("Disable the file indexer"));
    parser.addPositionalArgument(QLatin1String("start"), i18n("Start the file indexer"));
    parser.addPositionalArgument(QLatin1String("stop"), i18n("Stop the file indexer"));
    parser.addPositionalArgument(QLatin1String("restart"), i18n("Restart the file indexer"));

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QTextStream err(stderr);
    QTextStream out(stdout);

    QString command = parser.positionalArguments().first();
    if (command == QLatin1String("status")) {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bool running = bus.interface()->isServiceRegistered(QLatin1String("org.kde.baloo.file"));

        if (running) {
            out << "Baloo File Indexer is running\n";
        }
        else {
            out << "Baloo File Indexer is NOT running\n";
        }

        const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file/");

        XapianDatabase database(path);
        Xapian::Database* xdb = database.db();
        Xapian::Enquire enquire(*xdb);

        enquire.set_query(Xapian::Query("Z1"));
        Xapian::MSet mset = enquire.get_mset(0, 10000000);
        int phaseOne = mset.size();

        enquire.set_query(Xapian::Query("Z2"));
        mset = enquire.get_mset(0, 10000000);
        int phaseTwo = mset.size();

        enquire.set_query(Xapian::Query("Z-1"));
        mset = enquire.get_mset(0, 10000000);
        int failed = mset.size();

        int total = xdb->get_doccount();

        out << "Indexed " << phaseTwo << " / " << total << " files\n";
        out << "Failed to index " << failed << " files\n";
        if (failed) {
            out << "File IDs: ";
            Xapian::MSetIterator iter = mset.begin();
            for (; iter != mset.end(); ++iter) {
                out << *iter << " ";
            }
            out << "\n";
        }

        int actualTotal = phaseOne + phaseTwo + failed;
        if (actualTotal != total) {
            out << total - actualTotal << " files not accounted for\n";
        }
        return 0;
    }

    if (command == QLatin1String("enable") || command == QLatin1String("disable")) {
        bool isEnabled = false;
        if (command == QLatin1String("enable")) {
            isEnabled = true;
        }
        else if (command == QLatin1String("disable")) {
            isEnabled = false;
        }

        KConfig config(QLatin1String("baloofilerc"));
        KConfigGroup basicSettings = config.group("Basic Settings");
        basicSettings.writeEntry("Indexing-Enabled", isEnabled);

        if (isEnabled) {
            out << "Enabling the File Indexer\n";
            config.group("General").writeEntry("first run", true);

            start();
        }
        else {
            out << "Disabling the File Indexer\n";

            stop();
            const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_cleaner"));
            QProcess::startDetached(exe);
        }

        return 0;
    }

    if (command == QLatin1String("start") || command == QLatin1String("stop") ||
        command == QLatin1String("restart")) {
        bool shouldStart = false;
        bool shouldStop = false;

        if (command == QLatin1String("start"))
            shouldStart = true;
        else if (command == QLatin1String("stop"))
            shouldStop = true;
        else if (command == QLatin1String("restart")) {
            shouldStart = true;
            shouldStop = true;
        }

        if (shouldStop)
            stop();
        if (shouldStart)
            start();
    }

    if (command == QStringLiteral("fileStatistics")) {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file/");

        XapianDatabase database(path);
        FileStatistics stats(database);
        stats.compute();
        stats.print();
    }
    return 0;
}
