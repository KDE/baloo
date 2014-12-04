/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <QObject>
#include <QTest>
#include <QTemporaryDir>
#include <QUuid>
#include <QTemporaryFile>

#include "filemapping.h"
#include "xapiandatabase.h"
#include "xapiandocument.h"
#include "basicindexingjob.h"

#include <QDebug>

class FileMappingBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

using namespace Baloo;

void FileMappingBenchmark::test()
{
    QTemporaryDir tempDir;

    XapianDatabase db(tempDir.path(), true);
    static int limit = 5000;
    for (int i = 0; i < limit; i++) {
        QTemporaryFile file;
        file.open();

        BasicIndexingJob job(FileMapping(file.fileName()), QString("text/plain"), false);
        job.index();

        db.addDocument(job.document());
    }
    db.commit();


    QBENCHMARK {
        for (int i = 1; i <= 5000; i++) {
            FileMapping map(i);
            map.fetch(&db);
        }
    }
}

QTEST_MAIN(FileMappingBenchmark)

#include "filemappingbenchmark.moc"
