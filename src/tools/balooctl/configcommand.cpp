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

#include "configcommand.h"
#include "indexerconfig.h"

#include <QTextStream>
#include <QFileInfo>

#include <KLocalizedString>

using namespace Baloo;

QString ConfigCommand::command()
{
    return QStringLiteral("config");
}

QString ConfigCommand::description()
{
    return i18n("Manipulate the Baloo configuration");
}

int ConfigCommand::exec(const QCommandLineParser& parser)
{
    QStringList args = parser.positionalArguments();
    args.removeFirst();

    QTextStream out(stdout);

    auto printCommand = [&out](const QString& command, const QString& description) {
        out << "  ";
        out.setFieldWidth(25);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << command;
        out.setFieldWidth(0);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << description << endl;
    };

    const QString command = args.isEmpty() ? QStringLiteral("help") : args.takeFirst();
    if (command == QStringLiteral("help")) {
        // Show help
        out << i18n("The config command can be used to manipulate the Baloo Configuration") << endl;
        out << i18n("Usage: balooctl config <command>") << endl << endl;
        out << i18n("Possible Commands:") << endl;


        printCommand(QStringLiteral("add"), i18n("Add a value to config parameter"));
        printCommand(QStringLiteral("rm | remove"), i18n("Remove a value from a config parameter"));
        printCommand(QStringLiteral("list | ls | show"), i18n("Show the value of a config parameter"));
        printCommand(QStringLiteral("set"), i18n("Set the value of a config parameter"));
        printCommand(QStringLiteral("help"), i18n("Display this help menu"));
        return 0;
    }

    if (command == QStringLiteral("list") || command == QStringLiteral("ls") || command == QStringLiteral("show")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be listed:") << endl << endl;

            printCommand(QStringLiteral("hidden"), i18n("Controls if Baloo indexes hidden files and folders"));
            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare("hidden", Qt::CaseInsensitive) == 0) {
            if (config.indexHidden()) {
                out << "yes" << endl;
            } else {
                out << "no" << endl;
            }

            return 0;
        }

        if (value.compare("includeFolders", Qt::CaseInsensitive) == 0) {
            const QStringList folders = config.includeFolders();
            for (const QString& folder: folders) {
                out << folder << endl;
            }
            return 0;
        }

        if (value.compare("excludeFolders", Qt::CaseInsensitive) == 0) {
            const QStringList folders = config.excludeFolders();
            for (const QString& folder: folders) {
                out << folder << endl;
            }
            return 0;
        }

        out << i18n("Config parameter could not be found") << endl;
        return 1;
    }

    if (command == QStringLiteral("rm") || command == QStringLiteral("remove")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:") << endl << endl;

            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare("includeFolders", Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided") << endl;
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist") << endl;
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory") << endl;
                return 1;
            }

            auto path = fileInfo.absoluteFilePath();
            QStringList folders = config.includeFolders();
            if (!folders.contains(path)) {
                out << path << " is not in the list of include folders" << endl;
                return 1;
            }

            folders.removeAll(path);
            config.setIncludeFolders(folders);

            return 0;
        }

        if (value.compare("excludeFolders", Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided") << endl;
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist") << endl;
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory") << endl;
                return 1;
            }

            auto path = fileInfo.absoluteFilePath();
            QStringList folders = config.excludeFolders();
            if (!folders.contains(path)) {
                out << path << " is not in the list of exclude folders" << endl;
                return 1;
            }

            folders.removeAll(path);
            config.setExcludeFolders(folders);

            return 0;
        }

        out << i18n("Config parameter could not be found") << endl;
        return 1;
    }

    if (command == QStringLiteral("add")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:") << endl << endl;

            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare("includeFolders", Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided") << endl;
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist") << endl;
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory") << endl;
                return 1;
            }

            auto path = fileInfo.absoluteFilePath();
            QStringList folders = config.includeFolders();
            if (folders.contains(path)) {
                out << path << " is already in the list of include folders" << endl;
                return 1;
            }

            for (const QString& folder : folders) {
                if (path.startsWith(folder)) {
                    out << "Parent folder " << folder << " is already in the list of include folders" << endl;
                    return 1;
                }
            }

            if (config.excludeFolders().contains(path)) {
                out << path << " is in the list of exclude folders" << endl;
                out << "Aborting" << endl;
            }

            folders.append(path);
            config.setIncludeFolders(folders);

            return 0;
        }

        if (value.compare("excludeFolders", Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided") << endl;
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist") << endl;
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory") << endl;
                return 1;
            }

            auto path = fileInfo.absoluteFilePath();
            QStringList folders = config.excludeFolders();
            if (folders.contains(path)) {
                out << path << " is already in the list of exclude folders" << endl;
                return 1;
            }

            for (const QString& folder : folders) {
                if (path.startsWith(folder)) {
                    out << "Parent folder " << folder << " is already in the list of exclude folders" << endl;
                    return 1;
                }
            }

            if (config.includeFolders().contains(path)) {
                out << path << " is in the list of exclude folders" << endl;
                out << "Aborting" << endl;
            }

            folders.append(path);
            config.setExcludeFolders(folders);

            return 0;
        }

        out << i18n("Config parameter could not be found") << endl;
        return 1;
    }

    if (command == QStringLiteral("set")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:") << endl << endl;

            printCommand(QStringLiteral("hidden"), i18n("Controls if Baloo indexes hidden files and folders"));
            return 0;
        }

        IndexerConfig config;
        QString configParam = args.takeFirst();

        if (configParam == QStringLiteral("hidden"))  {
            if (args.isEmpty()) {
                out << i18n("Must provide a value") << endl;
                return 1;
            }

            QString value = args.takeFirst();
            if (value.compare("true", Qt::CaseInsensitive) == 0
                    || value.compare("y", Qt::CaseInsensitive) == 0
                    || value.compare("yes", Qt::CaseInsensitive) == 0
                    || value.compare("1") == 0) {
                config.setIndexHidden(true);
                return 0;
            }

            if (value.compare("false", Qt::CaseInsensitive) == 0
                    || value.compare("n", Qt::CaseInsensitive) == 0
                    || value.compare("no", Qt::CaseInsensitive) == 0
                    || value.compare("0") == 0) {
                config.setIndexHidden(false);
                return 0;
            }

            out << i18n("Invalid value") << endl;
            return 1;
        }

        out << i18n("Config parameter could not be found") << endl;
        return 1;
    }

    return 0;
}
