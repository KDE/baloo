/*
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

#include "fileindexerconfigutils.h"

#include "../fileindexerconfig.h"
#include "../basicindexingqueue.h"
#include "../database.h"
#include "../../lib/autotests/xattrdetector.h"
#include "baloo_xattr_p.h"

#include <QTest>
#include <QSignalSpy>
#include <QEventLoop>

using namespace Baloo;

class BasicIndexingQueueTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSimpleDirectoryStructure();
    void textExtendedAttributeIndexing();
    void textNormalAndThenExtendedAttributeIndexing();
    void testExtendedAttributeIndexingWhenEmpty();
    void testFileModifications();
};

Q_DECLARE_METATYPE(Xapian::Document);

void BasicIndexingQueueTest::testSimpleDirectoryStructure()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");
    dirs << QLatin1String("home/2");
    dirs << QLatin1String("home/kde/");
    dirs << QLatin1String("home/kde/1");
    dirs << QLatin1String("home/docs/");
    dirs << QLatin1String("home/docs/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);
    QCOMPARE(queue.isSuspended(), false);

    QSignalSpy spy(&queue, SIGNAL(newDocument(quint64,Xapian::Document)));
    QSignalSpy spyStarted(&queue, SIGNAL(startedIndexing()));
    QSignalSpy spyFinished(&queue, SIGNAL(finishedIndexing()));

    queue.enqueue(FileMapping(dir->path() + QLatin1String("/home")));

    QEventLoop loop;
    connect(&queue, &BasicIndexingQueue::finishedIndexing, &loop, &QEventLoop::quit);
    loop.exec();

    // kde and kde/1 are not indexed
    QCOMPARE(spy.count(), 5);
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyFinished.count(), 1);

    QStringList urls;
    for (int i = 0; i < spy.count(); i++) {
        QVariantList args = spy.at(i);
        QCOMPARE(args.size(), 2);

        Xapian::Document doc = args[1].value<Xapian::Document>();
        urls << QString::fromUtf8(doc.get_value(3).c_str());
    }

    QString home = dir->path() + QLatin1String("/home");

    QStringList expectedUrls;
    expectedUrls << home << home + QLatin1String("/1") << home + QLatin1String("/2") << home + QLatin1String("/docs")
                 << home + QLatin1String("/docs/1");

    // Based on the locale the default sorting order could be different.
    // Plus, we don't care about the order. We just want the same files
    // to be indexed
    QCOMPARE(expectedUrls.size(), urls.size());
    Q_FOREACH (const QString& url, expectedUrls) {
        QVERIFY(urls.count(url) == 1);
    }
}


void BasicIndexingQueueTest::textExtendedAttributeIndexing()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);

    // Index the file once
    // Write xattr stuff
    XattrDetector detector;
    if (!detector.isSupported(dbDir.path())) {
        qWarning() << "Xattr not supported on this filesystem";
        return;
    }

    const QString fileName = dir->path() + QStringLiteral("/home/1");
    QString rat = QString::number(4);
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.baloo.rating"), rat) != -1);

    QStringList tags;
    tags << QLatin1String("TagA") << QLatin1String("TagB") << QLatin1String("Tag C") << QLatin1String("Tag/D");

    QString tagStr = tags.join(QLatin1String(","));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.tags"), tagStr) != -1);

    const QString userComment(QLatin1String("UserComment"));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), userComment) != -1);

    QSignalSpy spy(&queue, SIGNAL(newDocument(quint64,Xapian::Document)));

    queue.enqueue(FileMapping(fileName), Baloo::ExtendedAttributesOnly);

    QEventLoop loop;
    connect(&queue, &BasicIndexingQueue::finishedIndexing, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy.size(), 1);
    int id = spy.first().at(0).toInt();
    Xapian::Document doc = spy.first().at(1).value<Xapian::Document>();
    spy.clear();

    Xapian::TermIterator iter = doc.termlist_begin();
    QCOMPARE(*iter, std::string("Cusercomment"));
    ++iter;
    QCOMPARE(*iter, std::string("R4"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-TagA"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-TagB"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-Tag C"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-Tag/D"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtaga"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtagb"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtag"));
    ++iter;
    QCOMPARE(*iter, std::string("TAc"));
    ++iter;
    QCOMPARE(*iter, std::string("TAd"));
    ++iter;
    QCOMPARE(iter, doc.termlist_end());

    db.xapianDatabase()->replaceDocument(id, doc);
    db.xapianDatabase()->commit();

    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), QStringLiteral("noob")) != -1);

    queue.enqueue(FileMapping(fileName), Baloo::ExtendedAttributesOnly);
    loop.exec();

    {
        QCOMPARE(spy.size(), 1);
        Xapian::Document doc = spy.first().at(1).value<Xapian::Document>();
        spy.clear();

        iter = doc.termlist_begin();
        QCOMPARE(*iter, std::string("Cnoob"));
        ++iter;
        QCOMPARE(*iter, std::string("R4"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-TagA"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-TagB"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-Tag C"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-Tag/D"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtaga"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtagb"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtag"));
        ++iter;
        QCOMPARE(*iter, std::string("TAc"));
        ++iter;
        QCOMPARE(*iter, std::string("TAd"));
        ++iter;
        QCOMPARE(iter, doc.termlist_end());
    }
}

void BasicIndexingQueueTest::textNormalAndThenExtendedAttributeIndexing()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);

    // Index the file once
    // Write xattr stuff
    XattrDetector detector;
    if (!detector.isSupported(dbDir.path())) {
        qWarning() << "Xattr not supported on this filesystem";
        return;
    }

    const QString fileName = dir->path() + QStringLiteral("/home/1");
    QString rat = QString::number(4);
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.baloo.rating"), rat) != -1);

    QStringList tags;
    tags << QLatin1String("TagA") << QLatin1String("TagB") << QLatin1String("Tag C") << QLatin1String("Tag/D");

    QString tagStr = tags.join(QLatin1String(","));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.tags"), tagStr) != -1);

    const QString userComment(QLatin1String("UserComment"));
    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), userComment) != -1);

    QSignalSpy spy(&queue, SIGNAL(newDocument(quint64,Xapian::Document)));

    queue.enqueue(FileMapping(fileName));

    QEventLoop loop;
    connect(&queue, &BasicIndexingQueue::finishedIndexing, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy.size(), 1);
    int id = spy.first().at(0).toInt();
    Xapian::Document doc = spy.first().at(1).value<Xapian::Document>();
    spy.clear();

    Xapian::TermIterator iter = doc.termlist_begin();
    iter.skip_to("C");
    QCOMPARE(*iter, std::string("Cusercomment"));
    iter.skip_to("R");
    QCOMPARE(*iter, std::string("R4"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-TagA"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-TagB"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-Tag C"));
    ++iter;
    QCOMPARE(*iter, std::string("TAG-Tag/D"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtaga"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtagb"));
    ++iter;
    QCOMPARE(*iter, std::string("TAtag"));
    ++iter;
    QCOMPARE(*iter, std::string("TAc"));
    ++iter;
    QCOMPARE(*iter, std::string("TAd"));

    db.xapianDatabase()->replaceDocument(id, doc);
    db.xapianDatabase()->commit();

    QVERIFY(baloo_setxattr(fileName, QLatin1String("user.xdg.comment"), QStringLiteral("noob")) != -1);

    queue.enqueue(FileMapping(fileName), Baloo::ExtendedAttributesOnly);
    loop.exec();

    {
        QCOMPARE(spy.size(), 1);
        Xapian::Document doc = spy.first().at(1).value<Xapian::Document>();
        spy.clear();

        iter = doc.termlist_begin();
        QCOMPARE(*iter, std::string("Cnoob"));
        ++iter;
        QCOMPARE(*iter, std::string("R4"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-TagA"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-TagB"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-Tag C"));
        ++iter;
        QCOMPARE(*iter, std::string("TAG-Tag/D"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtaga"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtagb"));
        ++iter;
        QCOMPARE(*iter, std::string("TAtag"));
        ++iter;
        QCOMPARE(*iter, std::string("TAc"));
        ++iter;
        QCOMPARE(*iter, std::string("TAd"));
        ++iter;
        QCOMPARE(iter, doc.termlist_end());
    }

}

void BasicIndexingQueueTest::testExtendedAttributeIndexingWhenEmpty()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);

    const QString fileName = dir->path() + QStringLiteral("/home/1");

    QSignalSpy spy(&queue, SIGNAL(newDocument(quint64,Xapian::Document)));
    queue.enqueue(FileMapping(fileName), Baloo::ExtendedAttributesOnly);

    QEventLoop loop;
    connect(&queue, &BasicIndexingQueue::finishedIndexing, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy.size(), 0);
}

void BasicIndexingQueueTest::testFileModifications()
{
    qRegisterMetaType<Xapian::Document>("Xapian::Document");

    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;
    BasicIndexingQueue queue(&db, &config);

    const QString fileName = dir->path() + QStringLiteral("/home/1");

    QSignalSpy spy(&queue, SIGNAL(newDocument(quint64,Xapian::Document)));
    queue.enqueue(FileMapping(fileName), Baloo::AutoUpdateFolder);

    QEventLoop loop;
    connect(&queue, &BasicIndexingQueue::finishedIndexing, &loop, &QEventLoop::quit);
    loop.exec();

    QCOMPARE(spy.size(), 1);
    spy.takeFirst();

    // Again
    queue.enqueue(FileMapping(fileName), Baloo::AutoUpdateFolder);
    loop.exec();

    QCOMPARE(spy.size(), 1);
}


QTEST_MAIN(BasicIndexingQueueTest)

#include "basicindexingqueuetest.moc"
