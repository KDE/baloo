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

#include "filewatch.h"
#include "fileindexerconfigutils.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "pendingfilequeue.h"

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <KFileMetaData/UserMetaData>

namespace Baloo {

class FileWatchTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testFileCreation();
    void testConfigChange();
};

}

using namespace Baloo;

namespace {
    bool createFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::WriteOnly);
        f1.close();
        return QFile::exists(fileUrl);
    }

    void modifyFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::Append | QIODevice::Text);

        QTextStream stream(&f1);
        stream << "1";
    }
}

void FileWatchTest::testFileCreation()
{
    QTemporaryDir includeDir;

    QStringList includeFolders;
    includeFolders << includeDir.path();

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db(dbDir.path());
    db.open(Baloo::Database::CreateDatabase);

    FileIndexerConfig config;

    FileWatch fileWatch(&db, &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, SIGNAL(installedWatches()));
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.count() || spy.wait());

    QSignalSpy spyIndexNew(&fileWatch, SIGNAL(indexNewFile(QString)));
    QSignalSpy spyIndexModified(&fileWatch, SIGNAL(indexModifiedFile(QString)));
    QSignalSpy spyIndexXattr(&fileWatch, SIGNAL(indexXAttr(QString)));

    QVERIFY(spyIndexNew.isValid());
    QVERIFY(spyIndexModified.isValid());
    QVERIFY(spyIndexXattr.isValid());

    // Create a file and see if it is indexed
    QString fileUrl(includeDir.path() + "/t1");
    QVERIFY(createFile(fileUrl));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 0);

    spyIndexNew.clear();
    spyIndexModified.clear();
    spyIndexXattr.clear();

    //
    // Modify the file
    //
    modifyFile(fileUrl);

    QVERIFY(spyIndexModified.wait());
    QCOMPARE(spyIndexNew.count(), 0);
    QCOMPARE(spyIndexModified.count(), 1);
    QCOMPARE(spyIndexXattr.count(), 0);

    spyIndexNew.clear();
    spyIndexModified.clear();
    spyIndexXattr.clear();

    //
    // Set an Xattr
    //
    KFileMetaData::UserMetaData umd(fileUrl);
    if (!umd.isSupported()) {
        qWarning() << "Xattr not supported on this filesystem:" << fileUrl;
        return;
    }

    const QString userComment(QStringLiteral("UserComment"));
    QVERIFY(umd.setUserComment(userComment) == KFileMetaData::UserMetaData::NoError);

    QVERIFY(spyIndexXattr.wait());
    QCOMPARE(spyIndexNew.count(), 0);
    QCOMPARE(spyIndexModified.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 1);

    spyIndexNew.clear();
    spyIndexModified.clear();
    spyIndexXattr.clear();
}

void FileWatchTest::testConfigChange()
{
    QTemporaryDir tmpDir;

    Database db(tmpDir.path());
    db.open(Baloo::Database::CreateDatabase);

    QString d1 = tmpDir.path() + QStringLiteral("/d1");
    QString d2 = tmpDir.path() + QStringLiteral("/d2");
    QString d11 = tmpDir.path() + QStringLiteral("/d1/d11");
    QString d21 = tmpDir.path() + QStringLiteral("/d2/d21");
    QString d22 = tmpDir.path() + QStringLiteral("/d2/d22");

    QDir().mkpath(d11);
    QDir().mkpath(d21);
    QDir().mkpath(d22);

    // parameters: includeFolders list, excludeFolders list
    Test::writeIndexerConfig({d1, d2}, {d11, d21});
    FileIndexerConfig config;

    FileWatch fileWatch(&db, &config);
    fileWatch.m_pendingFileQueue->setMaximumTimeout(0);
    fileWatch.m_pendingFileQueue->setMinimumTimeout(0);
    fileWatch.m_pendingFileQueue->setTrackingTime(0);

    QSignalSpy spy(&fileWatch, SIGNAL(installedWatches()));
    QVERIFY(spy.isValid());

    fileWatch.updateIndexedFoldersWatches();
    QVERIFY(spy.count() || spy.wait());

    QSignalSpy spyIndexNew(&fileWatch, SIGNAL(indexNewFile(QString)));
    QVERIFY(spyIndexNew.isValid());
    QVERIFY(createFile(d1 + "/t1"));
    QVERIFY(createFile(d2 + "/t2"));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 2);
    spyIndexNew.clear();

    // dir d22 is not yet excluded, so one event is expected
    QVERIFY(createFile(d11 + "/tx1"));
    QVERIFY(createFile(d21 + "/tx2"));
    QVERIFY(createFile(d22 + "/tx3"));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QList<QVariant> event = spyIndexNew.at(0);
    QCOMPARE(event.at(0).toString(), d22 + "/tx3");
    spyIndexNew.clear();

    Test::writeIndexerConfig({d2}, {d22});
    config.forceConfigUpdate();
    fileWatch.updateIndexedFoldersWatches();

    // dir d1 is no longer included
    QVERIFY(createFile(d1 + "/tx1a"));
    QVERIFY(createFile(d2 + "/tx2a"));
    QVERIFY(spyIndexNew.wait());
    QList<QString> result;
    for (const QList<QVariant>& event : qAsConst(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d2 + "/tx2a"});
    spyIndexNew.clear();
    result.clear();

    // d11 is implicitly excluded, as d1 is no longer included
    // d22 is explicitly excluded now, d21 is included
    QVERIFY(createFile(d11 + "/tx1b"));
    QVERIFY(createFile(d21 + "/tx2b"));
    QVERIFY(createFile(d22 + "/tx3b"));

    QVERIFY(spyIndexNew.wait(500));
    for (const QList<QVariant>& event : qAsConst(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d21 + "/tx2b"});
}

QTEST_MAIN(FileWatchTest)

#include "filewatchtest.moc"
