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

#include <KDebug>
#include <KTempDir>
#include <QTime>
#include <QCoreApplication>

#include "../basicindexingqueue.h"
#include "../commitqueue.h"
#include "../database.h"
#include "../fileindexerconfig.h"
#include "../lib/filemapping.h"

int main(int argc, char** argv)
{
    KTempDir tempDir;

    Database db;
    db.setPath(tempDir.name());
    db.init();

    Baloo::FileIndexerConfig config;
    QCoreApplication app(argc, argv);

    Baloo::BasicIndexingQueue basicIQ(&db, &config);
    QObject::connect(&basicIQ, SIGNAL(finishedIndexing()), &app, SLOT(quit()));

    Baloo::CommitQueue commitQueue(&db);
    QObject::connect(&basicIQ, SIGNAL(newDocument(uint,Xapian::Document)),
                     &commitQueue, SLOT(add(uint,Xapian::Document)));

    basicIQ.enqueue(Baloo::FileMapping(QDir::homePath()));

    QTime timer;
    timer.start();
    int ret = app.exec();

    commitQueue.commit();
    qDebug() << "Elapsed:" << timer.elapsed();

    // Print the io usage
    QFile file("/proc/self/io");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString str = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&str);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar("rchar: ");
        if (str.startsWith(rchar)) {
            ulong amt = str.mid(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024  << "kb";
        }

        QString wchar("wchar: ");
        if (str.startsWith(wchar)) {
            ulong amt = str.mid(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024  << "kb";
        }

        QString read("read_bytes: ");
        if (str.startsWith(read)) {
            ulong amt = str.mid(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024  << "kb";
        }

        QString write("write_bytes: ");
        if (str.startsWith(write)) {
            ulong amt = str.mid(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024  << "kb";
        }
    }
    qDebug() << "\nThe actual read/writes may be 0 because of an existing"
             << "cache and /tmp being memory mapped";

    return ret;
}
