/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QMimeDatabase>
#include <QTemporaryDir>

#include "database.h"
#include "src/file/basicindexingjob.h"
#include "transaction.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tempDir;
    qDebug() << tempDir.path();

    Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    Transaction tr(db, Transaction::ReadWrite);

    QMimeDatabase mimeDb;
    {
        QElapsedTimer timer;
        timer.start();

        QDirIterator it(QDir::homePath(), QDirIterator::Subdirectories);
        uint num = 0;
        while (it.hasNext()) {
            const QString& path = it.next();
            const QString& mimetype = mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();

            BasicIndexingJob job(path, mimetype);
            job.index();

            tr.addDocument(job.document());
            num++;

            if ((num % 10000) == 0) {
                qDebug() << num;
            }
        }
        tr.commit();

        qDebug() << "Done" << timer.elapsed() << "msecs";
        app.exec();
    }

    return 0;
}
