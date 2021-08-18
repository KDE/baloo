/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    QString fileUrl(includeDir.path() + QStringLiteral("/t1"));
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
    QVERIFY(createFile(d1 + QStringLiteral("/t1")));
    QVERIFY(createFile(d2 + QStringLiteral("/t2")));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 2);
    spyIndexNew.clear();

    // dir d22 is not yet excluded, so one event is expected
    QVERIFY(createFile(d11 + QStringLiteral("/tx1")));
    QVERIFY(createFile(d21 + QStringLiteral("/tx2")));
    QVERIFY(createFile(d22 + QStringLiteral("/tx3")));

    QVERIFY(spyIndexNew.wait());
    QCOMPARE(spyIndexNew.count(), 1);
    QList<QVariant> event = spyIndexNew.at(0);
    QCOMPARE(event.at(0).toString(), d22 + QStringLiteral("/tx3"));
    spyIndexNew.clear();

    Test::writeIndexerConfig({d2}, {d22});
    config.forceConfigUpdate();
    fileWatch.updateIndexedFoldersWatches();

    // dir d1 is no longer included
    QVERIFY(createFile(d1 + QStringLiteral("/tx1a")));
    QVERIFY(createFile(d2 + QStringLiteral("/tx2a")));
    QVERIFY(spyIndexNew.wait());
    QList<QString> result;
    for (const QList<QVariant>& event : std::as_const(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d2 + QStringLiteral("/tx2a")});
    spyIndexNew.clear();
    result.clear();

    // d11 is implicitly excluded, as d1 is no longer included
    // d22 is explicitly excluded now, d21 is included
    QVERIFY(createFile(d11 + QStringLiteral("/tx1b")));
    QVERIFY(createFile(d21 + QStringLiteral("/tx2b")));
    QVERIFY(createFile(d22 + QStringLiteral("/tx3b")));

    QVERIFY(spyIndexNew.wait(500));
    for (const QList<QVariant>& event : std::as_const(spyIndexNew)) {
	result.append(event.at(0).toString());
    }
    QCOMPARE(result, {d21 + QStringLiteral("/tx2b")});
}

QTEST_MAIN(FileWatchTest)

#include "filewatchtest.moc"
