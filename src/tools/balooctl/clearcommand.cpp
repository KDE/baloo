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

#include "clearcommand.h"
#include "clearentry.h"

using namespace Baloo;

QString ClearCommand::command()
{
    return QStringLiteral("clear");
}

QString ClearCommand::description()
{
    return i18n("Immediately clear entries from the index");
}

//  ClearCommand::exec
//  Sets up output stream and calls clearEntryNow for each of the arguments

int ClearCommand::exec(const QCommandLineParser &parser)
{
    QTextStream out(stdout);

    if (parser.positionalArguments().size() < 2) {
        out << "Enter a filename to index\n";
        return 1;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadWriteDatabase)) {
        out << "Baloo Index could not be opened\n";
        return 1;
    }

    ClearEntry collection(db, out);

    for (int i = 1; i < parser.positionalArguments().size(); ++i) {
        collection.clearEntryNow(parser.positionalArguments().at(i));
    }

    collection.commit();
    out << "File(s) cleared\n";

    return 0;
}
