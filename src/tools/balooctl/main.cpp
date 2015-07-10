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

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KFormat>
#include <QStandardPaths>
#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QLocale>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "database.h"
#include "transaction.h"
#include "databasesize.h"

#include "indexer.h"
#include "idutils.h"
#include "fileindexerconfig.h"
#include "monitor.h"
#include "baloo_interface.h"

using namespace Baloo;

void start()
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file"));
    QProcess::startDetached(exe);
}

int main(int argc, char* argv[])
{
    KAboutData aboutData(QLatin1String("balooctl"), i18n("balooctl"), PROJECT_VERSION);
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
    parser.addPositionalArgument(QLatin1String("suspend"), i18n("Suspend the file indexer"));
    parser.addPositionalArgument(QLatin1String("resume"), i18n("Resume the file indexer"));
    parser.addPositionalArgument(QLatin1String("check"), i18n("Check for any unindexed files and index them"));
    parser.addPositionalArgument(QLatin1String("index"), i18n("Index the specified files"));

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QTextStream err(stderr);
    QTextStream out(stdout);

    qRegisterMetaType<IndexerState>("IndexerState");
    qDBusRegisterMetaType<IndexerState>();

    QString command = parser.positionalArguments().first();

    OrgKdeBalooInterface balooInterface(QStringLiteral("org.kde.baloo"),
                                        QStringLiteral("/indexer"),
                                        QDBusConnection::sessionBus());

    if (command == QLatin1String("status")) {

        const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
        Database db(path);
        if (!db.open(Baloo::Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadOnly);

        if (parser.positionalArguments().length() == 1) {

            bool running = balooInterface.isValid();

            if (running) {
                out << "Baloo File Indexer is running\n";
                qDebug() << balooInterface.state();
                out << "Indexer state: " << stateString(balooInterface.state()) << endl;
            }
            else {
                out << "Baloo File Indexer is NOT running\n";
            }

            uint phaseOne = tr.phaseOneSize();
            uint total = tr.size();

            out << "Indexed " << total - phaseOne << " / " << total << " files\n";

            QFileInfo indexInfo(path + QLatin1String("index"));
            quint32 size = indexInfo.size();
            KFormat format(QLocale::system());
            if (size) {
                out << "Current size of index is " << format.formatByteSize(size, 2) << endl;
            } else {
                out << "Index does not exist yet\n";
            }
        } else {
            FileIndexerConfig m_config;

            for (int i = 1; i < parser.positionalArguments().length(); ++i) {
                QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
                quint64 id = filePathToId(QFile::encodeName(url));

                out << "File: " << url << endl;

                out << "Basic indexing: ";
                if (tr.hasDocument(id)) {
                    out << "done\n";
                } else if (m_config.shouldBeIndexed(url)) {
                    out << "scheduled\n";
                    return 0;
                } else {
                    out << "disbabled\n";
                    return 0;
                }

                out << "Content indexing: ";
                if (tr.inPhaseOne(id)) {
                    out << "scheduled\n";
                } else if (!tr.documentData(id).isEmpty()) {
                    out << "done\n";
                } else if (tr.hasFailed(id)) {
                    out << "failed\n";
                } else {
                    out << "disabled\n";
                }
            }
        }

            /*
            if (failed) {
                out << "Failed to index " << failed << " files\n";
                out << "File IDs: ";
                Xapian::MSetIterator iter = mset.begin();
                for (; iter != mset.end(); ++iter) {
                    out << *iter << " ";
                }
                out << "\n";
            }
            */

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

            balooInterface.quit();
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
            balooInterface.quit();
        if (shouldStart)
            start();
    }

    if (command == QStringLiteral("suspend")) {
        balooInterface.suspend();
        out << "File Indexer suspended\n";
        return 0;
    }

    if (command == QStringLiteral("resume")) {
        balooInterface.resume();
        out << "File Indexer resumed\n";
        return 0;
    }

    if (command == QStringLiteral("check")) {
        //TODO: implement in new arch
        //balooInterface.updateAllFolders();
        out << "Started search for unindexed files\n";
        return 0;
    }

    if (command == QStringLiteral("index")) {
        if (parser.positionalArguments().size() < 2) {
            out << "Please enter a filename to index\n";
            return 1;
        }

        const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");

        Database db(path);
        if (!db.open(Baloo::Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadWrite);

        for (int i = 1; i < parser.positionalArguments().size(); ++i) {
            const QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(url));
            if (id == 0) {
                out << "Could not stat file: " << url;
                continue;
            }
            if (tr.inPhaseOne(id))  {
                out << "Skipping: " << url << " Reason: Already scheduled for indexing\n";
                continue;
            }
            if (!tr.documentData(id).isEmpty()) {
                out << "Skipping: " << url << " Reason: Already indexed\n";
            }
            Indexer indexer(url, &tr);
            out << "Indexing " << url << endl;
            indexer.index();
        }
        tr.commit();
        out << "File(s) indexed\n";
    }

    if (command == QStringLiteral("indexSize")) {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/");
        Database db(path);

        if (!db.open(Baloo::Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        DatabaseSize size;
        {
            Transaction tr(db, Transaction::ReadOnly);
            size = tr.dbSize();
        }

        KFormat format(QLocale::system());
        auto prFunc = [&](const QString& name, uint size, uint totalSize) {
            out.setFieldWidth(20);
            out << name;
            out.setFieldWidth(0);
            out << ":";
            out.setFieldWidth(15);
            out << format.formatByteSize(size, 2);
            out.setFieldWidth(10);
            out << QString::number((100.0 * size / totalSize), 'f', 3);
            out.setFieldWidth(0);
            out << " %\n";
        };

        out << "Size: " << format.formatByteSize(size.size, 2) << "\n\n";
        prFunc("PostingDB", size.postingDb, size.size);
        prFunc("PosistionDB", size.positionDb, size.size);
        prFunc("DocTerms", size.docTerms, size.size);
        prFunc("DocFilenameTerms", size.docFilenameTerms, size.size);
        prFunc("DocXattrTerms", size.docXattrTerms, size.size);
        prFunc("IdTree", size.idTree, size.size);
        prFunc("IdFileName", size.idFilename, size.size);
        prFunc("DocTime", size.docTime, size.size);
        prFunc("DocData", size.docData, size.size);
        prFunc("ContentIndexingDB", size.contentIndexingIds, size.size);
        prFunc("FailedIdsDB", size.failedIds, size.size);
        prFunc("MTimeDB", size.mtimeDb, size.size);

        return 0;
    }

    if (command == QStringLiteral("monitor")) {
        Monitor mon;
        app.exec();
    }

    return 0;
}
