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

#include "global.h"
#include "database.h"
#include "transaction.h"
#include "databasesize.h"

#include "indexer.h"
#include "indexerconfig.h"
#include "idutils.h"
#include "fileindexerconfig.h"
#include "monitorcommand.h"
#include "schedulerinterface.h"
#include "maininterface.h"
#include "indexerstate.h"
#include "configcommand.h"
#include "statuscommand.h"

using namespace Baloo;

void start()
{
    const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
    QProcess::startDetached(exe);
}

int main(int argc, char* argv[])
{
    KAboutData aboutData(QStringLiteral("baloo"), i18n("balooctl"), PROJECT_VERSION);
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QStringLiteral("vhanda@kde.org"));

    QCoreApplication app(argc, argv);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("command"), i18n("The command to execute"));

    parser.addPositionalArgument(QStringLiteral("status"), i18n("Print the status of the indexer"));
    parser.addPositionalArgument(QStringLiteral("enable"), i18n("Enable the file indexer"));
    parser.addPositionalArgument(QStringLiteral("disable"), i18n("Disable the file indexer"));
    parser.addPositionalArgument(QStringLiteral("start"), i18n("Start the file indexer"));
    parser.addPositionalArgument(QStringLiteral("stop"), i18n("Stop the file indexer"));
    parser.addPositionalArgument(QStringLiteral("restart"), i18n("Restart the file indexer"));
    parser.addPositionalArgument(QStringLiteral("suspend"), i18n("Suspend the file indexer"));
    parser.addPositionalArgument(QStringLiteral("resume"), i18n("Resume the file indexer"));
    parser.addPositionalArgument(QStringLiteral("check"), i18n("Check for any unindexed files and index them"));
    parser.addPositionalArgument(QStringLiteral("index"), i18n("Index the specified files"));
    parser.addPositionalArgument(QStringLiteral("clear"), i18n("Forget the specified files"));
    parser.addPositionalArgument(QStringLiteral("config"), i18n("Modify the Baloo configuration"));
    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QTextStream err(stderr);
    QTextStream out(stdout);

    QString command = parser.positionalArguments().first();

    org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/"),
                                                QDBusConnection::sessionBus());

    org::kde::baloo::scheduler schedulerinterface(QStringLiteral("org.kde.baloo"),
                                        QStringLiteral("/scheduler"),
                                        QDBusConnection::sessionBus());

    if (command == QStringLiteral("config")) {
        ConfigCommand command;
        return command.exec(parser);
    }

    if (command == QLatin1String("status")) {
        StatusCommand command;
        return command.exec(parser);
    }

    if (command == QLatin1String("enable") || command == QLatin1String("disable")) {
        bool isEnabled = false;
        if (command == QLatin1String("enable")) {
            isEnabled = true;
        }
        else if (command == QLatin1String("disable")) {
            isEnabled = false;
        }

        IndexerConfig cfg;
        cfg.setFileIndexingEnabled(isEnabled);

        if (isEnabled) {
            out << "Enabling the File Indexer\n";

            cfg.setFirstRun(true);
            start();
        } else {
            out << "Disabling the File Indexer\n";

            mainInterface.quit();
            const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/baloo/index");
            QFile(path).remove();
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
            mainInterface.quit();
        if (shouldStart)
            start();
    }

    if (command == QStringLiteral("suspend")) {
        schedulerinterface.suspend();
        out << "File Indexer suspended\n";
        return 0;
    }

    if (command == QStringLiteral("resume")) {
        schedulerinterface.resume();
        out << "File Indexer resumed\n";
        return 0;
    }

    if (command == QStringLiteral("check")) {
        schedulerinterface.checkUnindexedFiles();
        out << "Started search for unindexed files\n";
        return 0;
    }

    if (command == QStringLiteral("index")) {
        if (parser.positionalArguments().size() < 2) {
            out << "Please enter a filename to index\n";
            return 1;
        }

        Database *db = globalDatabaseInstance();
        if (!db->open(Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadWrite);

        for (int i = 1; i < parser.positionalArguments().size(); ++i) {
            const QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(url));
            if (id == 0) {
                out << "Could not stat file: " << url << endl;
                continue;
            }
            if (tr.inPhaseOne(id))  {
                out << "Skipping: " << url << " Reason: Already scheduled for indexing\n";
                continue;
            }
            if (!tr.documentData(id).isEmpty()) {
                out << "Skipping: " << url << " Reason: Already indexed\n";
                continue;
            }
            Indexer indexer(url, &tr);
            out << "Indexing " << url << endl;
            indexer.index();
        }
        tr.commit();
        out << "File(s) indexed\n";
    }

    if (command == QStringLiteral("clear")) {
        if (parser.positionalArguments().size() < 2) {
            out << "Please enter a filename to index\n";
            return 1;
        }

        Database *db = globalDatabaseInstance();
        if (!db->open(Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadWrite);

        for (int i = 1; i < parser.positionalArguments().size(); ++i) {
            const QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(url));
            if (id == 0) {
                out << "Could not stat file: " << url << endl;
                continue;
            }
            if (tr.documentData(id).isEmpty()) {
                out << "Skipping: " << url << " Reason: Not yet indexed\n";
                continue;
            }
            Indexer indexer(url, &tr);
            out << "Clearing " << url << endl;
            tr.removeDocument(id);
        }
        tr.commit();
        out << "File(s) cleared\n";
    }

    if (command == QStringLiteral("indexSize")) {
        Database *db = globalDatabaseInstance();
        if (!db->open(Database::OpenDatabase)) {
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

        uint ts = size.expectedSize;
        out << "Actual Size: " << format.formatByteSize(size.actualSize, 2) << "\n";
        out << "Expected Size: " << format.formatByteSize(size.expectedSize, 2) << "\n\n";
        prFunc(QStringLiteral("PostingDB"), size.postingDb, ts);
        prFunc(QStringLiteral("PosistionDB"), size.positionDb, ts);
        prFunc(QStringLiteral("DocTerms"), size.docTerms, ts);
        prFunc(QStringLiteral("DocFilenameTerms"), size.docFilenameTerms, ts);
        prFunc(QStringLiteral("DocXattrTerms"), size.docXattrTerms, ts);
        prFunc(QStringLiteral("IdTree"), size.idTree, ts);
        prFunc(QStringLiteral("IdFileName"), size.idFilename, ts);
        prFunc(QStringLiteral("DocTime"), size.docTime, ts);
        prFunc(QStringLiteral("DocData"), size.docData, ts);
        prFunc(QStringLiteral("ContentIndexingDB"), size.contentIndexingIds, ts);
        prFunc(QStringLiteral("FailedIdsDB"), size.failedIds, ts);
        prFunc(QStringLiteral("MTimeDB"), size.mtimeDb, ts);

        return 0;
    }

    if (command == QStringLiteral("monitor")) {
        MonitorCommand mon;
        return mon.exec(parser);
    }

    if (command == QStringLiteral("checkDb")) {
        Database *db = globalDatabaseInstance();
        if (!db->open(Database::OpenDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadOnly);
        tr.checkPostingDbinTermsDb();
        tr.checkTermsDbinPostingDb();
        out << "Checking file paths .. " << endl;
        tr.checkFsTree();
        return 0;
    }

    parser.showHelp(1);
    return 0;
}
