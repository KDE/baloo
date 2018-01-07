/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "statuscommand.h"
#include "indexerconfig.h"

#include "global.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include "schedulerinterface.h"
#include "maininterface.h"
#include "indexerstate.h"

#include <KLocalizedString>
#include <KFormat>

using namespace Baloo;

QString StatusCommand::command()
{
    return QStringLiteral("status");
}

QString StatusCommand::description()
{
    return i18n("Print the status of the Indexer");
}

int StatusCommand::exec(const QCommandLineParser& parser)
{
    QTextStream out(stdout);

    IndexerConfig cfg;
    if (!cfg.fileIndexingEnabled()) {
        out << i18n("Baloo is currently disabled. To enable, please run \"balooctl enable\"") << endl;
        return 1;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadOnlyDatabase)) {
        out << i18n("Baloo Index could not be opened") << endl;
        return 1;
    }

    Transaction tr(db, Transaction::ReadOnly);

    QStringList args = parser.positionalArguments();
    args.pop_front();

    if (args.isEmpty()) {
        org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                    QStringLiteral("/"),
                                                    QDBusConnection::sessionBus());

        org::kde::baloo::scheduler schedulerinterface(QStringLiteral("org.kde.baloo"),
                                            QStringLiteral("/scheduler"),
                                            QDBusConnection::sessionBus());

        bool running = mainInterface.isValid();

        if (running) {
            out << i18n("Baloo File Indexer is running") << endl;
            out << i18n("Indexer state: %1", stateString(schedulerinterface.state())) << endl;
        }
        else {
            out << i18n("Baloo File Indexer is not running") << endl;
        }

        uint phaseOne = tr.phaseOneSize();
        uint total = tr.size();

        out << i18n("Indexed %1 / %2 files", total - phaseOne, total) << endl;

        const QString path = fileIndexDbPath();

        const QFileInfo indexInfo(path + QLatin1String("/index"));
        const auto size = indexInfo.size();
        KFormat format(QLocale::system());
        if (size) {
            out << "Current size of index is " << format.formatByteSize(size, 2) << endl;
        } else {
            out << "Index does not exist yet\n";
        }
    } else {
        for (const QString& arg : args) {
            QString filePath = QFileInfo(arg).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(filePath));

            out << i18n("File: %1", filePath) << endl;

            out << "Basic Indexing: ";
            if (tr.hasDocument(id)) {
                out << "Done\n";
            } else if (cfg.shouldBeIndexed(filePath)) {
                out << "Scheduled\n";
                continue;
            } else {
                // FIXME: Add why it is not being indexed!
                out << "Disabled\n";
                continue;
            }

            if (QFileInfo(arg).isDir()) {
                continue;
            }

            out << "Content Indexing: ";
            if (tr.inPhaseOne(id)) {
                out << "Scheduled\n";
            } else if (tr.hasFailed(id)) {
                out << "Failed\n";
            } else {
                out << "Done\n";
            }
        }
    }

    return 0;
}
