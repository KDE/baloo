/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015 Vishesh Handa <vhanda@kde.org>
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
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDirIterator>
#include <QDir>
#include <QTime>

#include "documenturldb.h"
#include "idutils.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    MDB_env* env;
    MDB_txn* txn;

    QTemporaryDir tempDir;
    qDebug() << tempDir.path();

    mdb_env_create(&env);
    mdb_env_set_maxdbs(env, 2);
    mdb_env_set_mapsize(env, 1048576000);

    // The directory needs to be created before opening the environment
    QByteArray path = QFile::encodeName(tempDir.path());
    mdb_env_open(env, path.constData(), MDB_NOMEMINIT | MDB_WRITEMAP, 0664);
    mdb_txn_begin(env, nullptr, 0, &txn);

    {
        DocumentUrlDB db(IdTreeDB::create(txn), IdFilenameDB::create(txn), txn);
        QTime timer;
        timer.start();

        QDirIterator it(QDir::homePath(), QDirIterator::Subdirectories);
        uint num = 0;
        while (it.hasNext()) {
            QByteArray path = QFile::encodeName(it.next());
            db.put(filePathToId(path), path);
            num++;

            if ((num % 10000) == 0) {
                qDebug() << num;
            }
        }
        mdb_txn_commit(txn);

        qDebug() << "Done" << timer.elapsed() << "msecs";
        app.exec();
    }

    // Cleanup
    //mdb_txn_abort(txn);
    mdb_env_close(env);

    return 0;
}
