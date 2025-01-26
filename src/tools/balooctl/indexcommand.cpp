/*
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QFile>

#include <KLocalizedString>
#include <QFileInfo>
#include <QTextStream>

#include "command.h"
#include "database.h"
#include "global.h"
#include "transaction.h"

#include "indexcommand.h"
#include "indexentry.h"

using namespace Baloo;

QString IndexCommand::command()
{
    return QStringLiteral("index");
}

QString IndexCommand::description()
{
    return i18n("Immediately index files");
}

//  IndexCommand::exec
//  Sets up stdout textstream and calls indexFileNow for each of the arguments

int IndexCommand::exec(const QCommandLineParser &parser)
{
    QTextStream out(stdout);

    if (parser.positionalArguments().size() < 2) {
        out << "Enter a filename to index\n";
        return 1;
    }

    Database *db = globalDatabaseInstance();
    if (db->open(Database::ReadWriteDatabase) != Database::OpenResult::Success) {
        out << "Baloo Index could not be opened\n";
        return 1;
    }

    IndexEntry collection(db, out);

    for (int i = 1; i < parser.positionalArguments().size(); ++i) {
        collection.indexFileNow(parser.positionalArguments().at(i));
    }

    collection.commit();
    out << "File(s) indexed\n";
    return 0;
}
