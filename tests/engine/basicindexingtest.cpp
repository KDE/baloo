/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <QDirIterator>
#include <QDir>
#include <QElapsedTimer>
#include <QMimeDatabase>

#include "database.h"
#include "databasesize.h"
#include "transaction.h"
#include "../file/util.h"
#include "src/file/basicindexingjob.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tempDir;
    qDebug() << "Creating temporary DB in" << tempDir.path();

    Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    Transaction tr(db, Transaction::ReadWrite);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({QStringLiteral("s"), QStringLiteral("Maximum transaction size (number of documents)"), QStringLiteral("size"), QStringLiteral("50000")});
    parser.addPositionalArgument(QStringLiteral("dir"), QStringLiteral("Index root directory"));
    parser.process(app);

    const uint transactionSize = parser.value(QStringLiteral("s")).toUInt();
    auto arguments = parser.positionalArguments();
    if (arguments.size() > 1 || !transactionSize) {
        parser.showHelp(1);
    }
    const QString path = [&arguments,&parser]() {
        if (arguments.empty()) {
            return QDir::homePath();
        } else {
            QFileInfo fi(arguments[0]);
            if (fi.isDir()) {
                return arguments[0];
            }
            parser.showHelp(1);
        }
    }();
    qDebug() << "Indexing documents in" << path << "\nTransaction size:" << transactionSize;

    {
        QMimeDatabase mimeDb;
        QElapsedTimer timer;
        timer.start();

        QDirIterator it(path, QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, QDirIterator::Subdirectories);
        uint num = 0;
        while (it.hasNext()) {
            const QString& path = it.next();
            const QString& mimetype = mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();

            BasicIndexingJob job(path, mimetype);
            if (!job.index()) {
                continue;
            }

            if (tr.hasDocument(job.document().id())) {
                qDebug() << "Skip" << path;
            } else {
              tr.addDocument(job.document());
              num++;
            }

            if ((num % transactionSize) == 0) {
                tr.commit();
                tr.reset(Transaction::ReadWrite);
                qDebug() << num << "- Commit";
            }
        }
        tr.commit();

        qDebug() << "Done -" << timer.elapsed() << "msecs," << num << "documents";
    }

    {
        Transaction tr(db, Transaction::ReadOnly);
        const auto dbSize = tr.dbSize();
        qDebug() << "File size (MiB):" << dbSize.actualSize / (1024.0 * 1024)
                 << "Used:" << dbSize.expectedSize / (1024.0 * 1024);
    }
    printIOUsage();

    return 0;
}
