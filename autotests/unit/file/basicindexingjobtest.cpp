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

#include "basicindexingjob.h"

#include <QMimeDatabase>
#include <QTest>
#include <QFileInfo>
#include <QDateTime>
#include <QTemporaryDir>
#include <QFile>

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
    /*
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
    XapianDocument docForNormal(jobNormal.document());

    // mimetype
    QCOMPARE(docForNormal.fetchTermStartsWith("M").remove(0, 1), mimetype);


    // filename
    QStringList terms = {"filename", "txt"};
    for (const QString& term: terms) {
        QCOMPARE(docForNormal.fetchTermStartsWith(term.toUtf8()), term);
        QByteArray prefix = "F";
        QByteArray termWithPrefix = prefix + term.toUtf8();
        QCOMPARE(docForNormal.fetchTermStartsWith(termWithPrefix).remove(0, prefix.size()), term);
    }

    // modified date
    QDateTime mod = fileInfo.lastModified();
    const QString dtm = "DT_M" % mod.toString(Qt::ISODate);
    const QString dtmY = "DT_MY" % QString::number(mod.date().year());
    const QString dtmM = "DT_MM" % QString::number(mod.date().month());
    const QString dtmD = "DT_MD" % QString::number(mod.date().day());

    QCOMPARE(docForNormal.fetchTermStartsWith("DT_M"), dtm);
    QCOMPARE(docForNormal.fetchTermStartsWith("DT_MY"), dtmY);
    QCOMPARE(docForNormal.fetchTermStartsWith("DT_MM"), dtmM);
    QCOMPARE(docForNormal.fetchTermStartsWith("DT_MD"), dtmD);

    const QString time_t = QString::fromStdString(docForNormal.doc().get_value(0));
    const QString julanDay = QString::fromStdString(docForNormal.doc().get_value(1));
    QCOMPARE(time_t, QString::number(mod.toTime_t()));
    QCOMPARE(julanDay, QString::number(mod.date().toJulianDay()));

    // types
    QVector<KFileMetaData::Type::Type> tList = BasicIndexingJob::typesForMimeType(mimetype);
    Q_FOREACH (KFileMetaData::Type::Type type, tList) {
        QByteArray prefix = "T";
        QString typeStr = KFileMetaData::TypeInfo(type).name().toLower();
        QByteArray typeWithPrefix = prefix + typeStr.toUtf8();
        QCOMPARE(docForNormal.fetchTermStartsWith(typeWithPrefix).remove(0, prefix.size()), typeStr);

    }

    // folder, basicindexing only
    if (fileInfo.isDir()) {
        QCOMPARE(docForNormal.fetchTermStartsWith("T"), QStringLiteral("T"));
    }

    QCOMPARE(docForNormal.fetchTermStartsWith("Z"), QStringLiteral("Z1"));

    // set up XapianDocument for basic indexing mode only
    BasicIndexingJob jobBasic(file, mimetype, true);
    jobBasic.index();
    XapianDocument docForBasic(jobBasic.document());
    QCOMPARE(docForBasic.fetchTermStartsWith("Z"), QStringLiteral("Z2"));
    */
}

QTEST_GUILESS_MAIN(BasicIndexingJobTest)

#include "basicindexingjobtest.moc"
