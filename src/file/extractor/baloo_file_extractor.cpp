/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010-14 Vishesh Handa <handa.vish@gmail.com>
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

#include "baloo_file_extractor.h"

#include <KLocalizedString>
#include <QStandardPaths>

#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../lib/extractorclient.h"

FileIndexWaiter::FileIndexWaiter(const QStringList &files, QObject *parent)
    : QObject(parent),
      m_files(files)
{
}

void FileIndexWaiter::fileIndexed(const QString &file)
{
    m_files.removeOne(file);
}

void FileIndexWaiter::dataSaved(const QString &file)
{
    if (m_files.isEmpty()) {
        qApp->quit();
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("Baloo File Extractor"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.1"));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("The File Extractor extracts the file metadata and text"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QLatin1String("urls"), i18n("The URL/id of the files to be indexed"));
    parser.addOption(QCommandLineOption(QLatin1String("debug"), i18n("Print the data being indexed")));
    parser.addOption(QCommandLineOption(QLatin1String("bdata"), i18n("Print the QVariantMap in Base64 encoding")));
    parser.addOption(QCommandLineOption(QLatin1String("ignoreConfig"), i18n("Ignore the baloofilerc config and always index the file")));

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/file");
    parser.addOption(QCommandLineOption(QLatin1String("db"), i18n("Specify a custom path for the database"),
                                        i18n("path"), path));

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        fprintf(stderr, "The url/id of the file is missing\n\n");
        parser.showHelp(1);
    }

    FileIndexWaiter *waiter = new FileIndexWaiter(args);
    Baloo::ExtractorClient *client = new Baloo::ExtractorClient;
    QObject::connect(client, &Baloo::ExtractorClient::extractorDied, &app, QCoreApplication::quit);
    QObject::connect(client, &Baloo::ExtractorClient::fileIndexed, waiter, &FileIndexWaiter::fileIndexed);
    QObject::connect(client, &Baloo::ExtractorClient::dataSaved, waiter, &FileIndexWaiter::dataSaved);
    if (parser.isSet(QLatin1String("bdata"))) {
        client->setBinaryOutput(true);
        client->setSaveToDatabase(false);
    }

    if (parser.isSet(QLatin1String("debug"))) {
        client->enableDebuging(true);
    }

    if (parser.isSet(QLatin1String("ignoreConfig"))) {
        client->setFollowConfig(false);
    }

    if (parser.isSet(QLatin1String("db"))) {
        client->setDatabasePath(parser.value(QLatin1String("db")));
    }

    for (const QString &file: args) {
        client->indexFile(file);
    }

    client->indexingComplete();
    return app.exec();
}
