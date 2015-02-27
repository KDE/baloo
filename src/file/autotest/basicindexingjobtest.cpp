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

#include "../basicindexingjob.h"
#include <lucene++/LuceneHeaders.h>
#include "lucenedocument.h"

#include <QMimeDatabase>
#include <QTest>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QTemporaryDir>
#include <QFile>
#include <KFileMetaData/TypeInfo>

namespace Baloo {

class BasicIndexingJobTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBasicIndexing();
};

}
using namespace Baloo;

void BasicIndexingJobTest::testBasicIndexing()
{
    QTemporaryDir dir;
    QFile qfile(dir.path() + "/" + "filename.txt");
    qfile.open(QIODevice::WriteOnly | QIODevice::Text);
    qfile.close();
    FileMapping file(qfile.fileName());
    QFileInfo fileInfo(qfile.fileName());

    QMimeDatabase m_mimeDb;
    QString mimetype = m_mimeDb.mimeTypeForFile(file.url(), QMimeDatabase::MatchExtension).name();

    bool onlyBasicIndexing = false;
    BasicIndexingJob jobNormal(file, mimetype, onlyBasicIndexing);
    jobNormal.index();
    LuceneDocument docForNormal(jobNormal.document());

    // mimetype
    QCOMPARE(docForNormal.getFieldValues(QStringLiteral("M")).at(0), mimetype);


    // filename
    // we are not tokenizing the name right now, as it will be done by lucene when adding doc to index
    QCOMPARE(docForNormal.getFieldValues("F").at(0), fileInfo.fileName());
    QCOMPARE(docForNormal.getFieldValues("content").at(0), fileInfo.fileName());
    // modified date
    QDateTime mod = fileInfo.lastModified();
    QCOMPARE(docForNormal.getFieldValues("DT_M").at(0), mod.toString(Qt::ISODate));
    QCOMPARE(docForNormal.getFieldValues("DT_MY").at(0), QString::number(mod.date().year()));
    QCOMPARE(docForNormal.getFieldValues("DT_MM").at(0), QString::number(mod.date().month()));
    QCOMPARE(docForNormal.getFieldValues("DT_MD").at(0), QString::number(mod.date().day()));
    QCOMPARE(docForNormal.getFieldValues("time_t").at(0), QString::number(mod.toTime_t()));
    QCOMPARE(docForNormal.getFieldValues("j_day").at(0), QString::number(mod.date().toJulianDay()));
    QCOMPARE(docForNormal.getFieldValues("CREATED").at(0), QString::number(fileInfo.created().toMSecsSinceEpoch()));

    QCOMPARE(docForNormal.getFieldValues("URL").at(0), file.url());

    // types
    QVector<KFileMetaData::Type::Type> tList = BasicIndexingJob::typesForMimeType(mimetype);
    QStringList types = docForNormal.getFieldValues("T");
    QStringList t_check;
    Q_FOREACH (KFileMetaData::Type::Type type, tList) {
        QString typeStr = KFileMetaData::TypeInfo(type).name().toLower();
        t_check.push_back(typeStr);
    }
    QCOMPARE(types, t_check);

    // folder, basicindexing only
    //TODO make a test in which this actually runs
    if (fileInfo.isDir()) {
        QCOMPARE(docForNormal.getFieldValues("Z"), QStringList() << "folder" << "2");
    }

    QCOMPARE(docForNormal.getFieldValues("Z").at(0), QStringLiteral("1"));

    // set up LuceneDocument for basic indexing mode only
    BasicIndexingJob jobBasic(file, mimetype, true);
    jobBasic.index();
    LuceneDocument docForBasic(jobBasic.document());
    QCOMPARE(docForBasic.getFieldValues("Z").at(0), QStringLiteral("2"));
}

QTEST_MAIN(BasicIndexingJobTest)

#include "basicindexingjobtest.moc"
