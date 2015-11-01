
/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014-2015 Vishesh Handa <vhanda@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDebug>
#include <QTemporaryDir>
#include <QTime>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "database.h"
#include "document.h"
#include "transaction.h"
#include "tests/file/util.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("num"), QStringLiteral("The number of terms. Each term is of length 10"));
    parser.addOption(QCommandLineOption(QStringList () << QStringLiteral("p") << QStringLiteral("position"), QStringLiteral("Add positional information")));
    parser.addHelpOption();
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(1);
    }

    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);

    Baloo::Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    qDebug() << tempDir.path();
    printIOUsage();
    qDebug() << "Creating the document";

    Baloo::Document doc;
    int size = args.first().toInt();

    for (int i = 0; i < size; i++) {
        QByteArray term = QUuid::createUuid().toByteArray().mid(1, 10);

        if (parser.isSet(QStringLiteral("p"))) {
            doc.addPositionTerm(term, i);
        }
        else {
            doc.addTerm(term);
        }
    }
    doc.setId(1);

    Baloo::Transaction tr(db, Baloo::Transaction::ReadWrite);
    tr.addDocument(doc);
    tr.commit();

    printIOUsage();

    int dbSize = 0;
    QDir dbDir(tempDir.path());
    for (const QFileInfo& file : dbDir.entryInfoList(QDir::Files)) {
        qDebug() << file.fileName() << file.size() / 1024 << "kb";
        dbSize += file.size();

    }
    qDebug() << "Database Size:" << dbSize / 1024 << "kb";

    return app.exec();
}
