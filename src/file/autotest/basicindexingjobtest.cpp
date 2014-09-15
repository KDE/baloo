/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA    02110-1301    USA
 *
 */

#include "basicindexingjobtest.h"

#include "../basicindexingjob.h"
#include <xapian.h>
#include "xapiandocument.h"

#include <QMimeDatabase>
#include <QTest>
#include <QTemporaryFile>

using namespace Baloo;

void BasicIndexingJobTest::testOnlyBasicIndexing()
{
    QTemporaryFile tempFile;
    tempFile.open();
    FileMapping file(tempFile.fileName());

    QMimeDatabase m_mimeDb;
    QString mimetype = m_mimeDb.mimeTypeForFile(file.url(), QMimeDatabase::MatchExtension).name();

    // test basic job
    BasicIndexingJob jobBasic(file, mimetype, true);
    jobBasic.index();
    XapianDocument docForBasic(jobBasic.document());
    QCOMPARE(docForBasic.fetchTermStartsWith("Z"), QStringLiteral("Z2"));
}

QTEST_MAIN(BasicIndexingJobTest)
