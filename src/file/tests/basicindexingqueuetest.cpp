/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
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
#include <QCoreApplication>

#include "basicindexingqueue.h"
#include "commitqueue.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "filemapping.h"
#include "util.h"

int main(int argc, char** argv)
{
    QTemporaryDir tempDir;

    Baloo::Database db(tempDir.path());
    //db.init();

    Baloo::FileIndexerConfig config;
    QCoreApplication app(argc, argv);

    Baloo::BasicIndexingQueue basicIQ(&db, &config);
    QObject::connect(&basicIQ, &Baloo::BasicIndexingQueue::finishedIndexing, &app, &QCoreApplication::quit);

    Baloo::CommitQueue commitQueue(&db);
    QObject::connect(&basicIQ, &Baloo::BasicIndexingQueue::newDocument, &commitQueue, &Baloo::CommitQueue::add);

    basicIQ.enqueue(Baloo::FileMapping(QDir::homePath()));

    QTime timer;
    timer.start();
    int ret = app.exec();

    commitQueue.commit();
    qDebug() << "Elapsed:" << timer.elapsed();

    printIOUsage();

    return ret;
}
