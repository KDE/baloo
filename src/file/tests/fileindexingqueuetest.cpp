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

#include "fileindexer.h"

#include "../basicindexingqueue.h"
#include "../commitqueue.h"
#include "../database.h"
#include "../fileindexerconfig.h"
#include "../lib/filemapping.h"

namespace {
    QString contents(const QString& url) {
        QFile file(url);
        file.open(QIODevice::ReadOnly);

        QTextStream stream(&file);
        return stream.readAll();
    }
}

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
    app.exec();

    commitQueue.commit();

    // Now the file indexing
    Xapian::Database* xdb = db.xapianDatabase()->db();
    Xapian::Enquire enquire(*xdb);
    enquire.set_query(Xapian::Query("Z1"));

    Xapian::MSet mset = enquire.get_mset(0, 50000);
    Xapian::MSetIterator it = mset.begin();

    QHash<QString, int> m_timePerType;
    QHash<QString, int> m_numPerType;

    uint totalTime = 0;
    for (; it != mset.end(); it++) {
        Baloo::FileMapping fileMap(*it);
        if (!fileMap.fetch(db.sqlDatabase()))
            continue;

        Baloo::FileIndexer* job = new Baloo::FileIndexer(fileMap.id(), fileMap.url());
        job->setCustomPath(db.path());
        job->exec();

        qDebug() << fileMap.id() << fileMap.url() << job->mimeType() << job->elapsed();
        totalTime += job->elapsed();

        m_timePerType[job->mimeType()] += job->elapsed();
        m_numPerType[job->mimeType()] += 1;
    }

    qDebug() << "\n\n";
    Q_FOREACH (const QString& type, m_timePerType.uniqueKeys()) {
        double averageTime = m_timePerType.value(type) / m_numPerType.value(type);
        qDebug() << type << averageTime;
    }
    qDebug() << "Total Elapsed:" << totalTime;
    return 0;
}
