/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2015 Vishesh Handa <vhanda@kde.org>

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

#include "unindexedfileiterator.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QElapsedTimer>

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tempDir;
    qDebug() << tempDir.path();

    Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    Transaction tr(db, Transaction::ReadWrite);

    FileIndexerConfig config;

    QElapsedTimer timer;
    timer.start();

    UnIndexedFileIterator it(&config, &tr, QDir::homePath());
    while (!it.next().isEmpty()) {
        ;
    }

    qDebug() << "Done" << timer.elapsed() << "msecs";
    return 0;
}
