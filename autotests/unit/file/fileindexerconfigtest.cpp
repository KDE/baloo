/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fileindexerconfig.h"
#include "fileindexerconfigutils.h"

#include <QTemporaryDir>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QScopedPointer>
#include <QTest>

using namespace Baloo::Test;

class FileIndexerConfigTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testShouldFolderBeIndexed();
    void testShouldBeIndexed();
    void testExcludeFilterOnFolders();
};

void FileIndexerConfigTest::testShouldFolderBeIndexed()
{
    // create the full folder hierarchy
    QTemporaryDir* mainDir = createTmpFolders(QStringList()
                                         << indexedRootDir
                                         << indexedSubDir
                                         << indexedSubSubDir
                                         << excludedSubSubDir
                                         << hiddenSubSubDir
                                         << ignoredSubFolderToIndexedHidden
                                         << indexedSubFolderToIndexedHidden
                                         << excludedRootDir
                                         << hiddenSubDir
                                         << indexedHiddenSubDir
                                         << ignoredRootDir
                                         << excludedRootDir);

    const QString dirPrefix = mainDir->path();

    // write the config
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + indexedSubFolderToIndexedHidden
                       << dirPrefix + indexedHiddenSubDir
                       << dirPrefix + indexedSubDirToExcluded,
                       QStringList()
                       << dirPrefix + excludedRootDir
                       << dirPrefix + excludedSubDir
                       << dirPrefix + excludedSubSubDir,
                       QStringList(),
                       false);

    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());

    // run through all the folders
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedRootDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDirToExcluded));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedHiddenSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredRootDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedRootDir));


    // re-write the config with hidden folder indexing enabled
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + indexedSubFolderToIndexedHidden
                       << dirPrefix + indexedHiddenSubDir
                       << dirPrefix + indexedSubDirToExcluded,
                       QStringList()
                       << dirPrefix + excludedRootDir
                       << dirPrefix + excludedSubDir
                       << dirPrefix + excludedSubSubDir,
                       QStringList(),
                       true);
    cfg->forceConfigUpdate();

    // run through all the folders
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedRootDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDirToExcluded));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedHiddenSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredRootDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedRootDir));


    // cleanup
    delete mainDir;
}

void FileIndexerConfigTest::testShouldBeIndexed()
{
    // create the full folder hierarchy
    QTemporaryDir* mainDir = createTmpFolders(QStringList()
                                         << indexedRootDir
                                         << indexedSubDir
                                         << indexedSubSubDir
                                         << excludedSubSubDir
                                         << hiddenSubSubDir
                                         << ignoredSubFolderToIndexedHidden
                                         << indexedSubFolderToIndexedHidden
                                         << excludedRootDir
                                         << hiddenSubDir
                                         << indexedHiddenSubDir
                                         << ignoredRootDir
                                         << excludedRootDir);

    const QString dirPrefix = mainDir->path();

    // write the config
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + indexedSubFolderToIndexedHidden
                       << dirPrefix + indexedHiddenSubDir
                       << dirPrefix + indexedSubDirToExcluded,
                       QStringList()
                       << dirPrefix + excludedRootDir
                       << dirPrefix + excludedSubDir
                       << dirPrefix + excludedSubSubDir,
                       QStringList(),
                       false);

    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());

    // run through all the folders
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedRootDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDirToExcluded));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedHiddenSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredRootDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedRootDir));

    // run through all the folders with a file name attached
    const QString fileName = QStringLiteral("/somefile.txt");
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedRootDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + hiddenSubSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubDirToExcluded + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + hiddenSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedHiddenSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + ignoredRootDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedRootDir + fileName));


    // re-write the config with hidden folder indexing enabled
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + indexedSubFolderToIndexedHidden
                       << dirPrefix + indexedHiddenSubDir
                       << dirPrefix + indexedSubDirToExcluded,
                       QStringList()
                       << dirPrefix + excludedRootDir
                       << dirPrefix + excludedSubDir
                       << dirPrefix + excludedSubSubDir,
                       QStringList(),
                       true);
    cfg->forceConfigUpdate();

    // run through all the folders
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedRootDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedSubDirToExcluded));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + hiddenSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedHiddenSubDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + ignoredRootDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedRootDir));

    // run through all the folders with a file name attached
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedRootDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + hiddenSubSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + ignoredSubFolderToIndexedHidden + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubFolderToIndexedHidden + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedSubDirToExcluded + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + hiddenSubDir + fileName));
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedHiddenSubDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + ignoredRootDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedRootDir + fileName));

    // cleanup
    delete mainDir;
}

void FileIndexerConfigTest::testExcludeFilterOnFolders()
{
    const QString excludeFilter1 = QStringLiteral("build");
    const QString excludeFilter2 = QStringLiteral("foo?ar");

    const QString excludedSubDir1 = indexedRootDir + QLatin1String("/") + excludeFilter1;
    const QString excludedSubSubDir1 = excludedSubDir1 + QLatin1String("/sub1");

    const QString excludedSubDir2 = indexedRootDir + QLatin1String("/foobar");
    const QString excludedSubSubDir2 = excludedSubDir2 + QLatin1String("/sub2");

    const QString includedSubDir = excludedSubDir1 + QLatin1String("/sub3");
    const QString includedSubSubDir = includedSubDir + QLatin1String("/sub");

    // create the full folder hierarchy
    QScopedPointer<QTemporaryDir> mainDir(createTmpFolders(QStringList()
                                     << indexedRootDir
                                     << excludedSubDir1
                                     << excludedSubSubDir1
                                     << excludedSubDir1
                                     << excludedSubSubDir2
                                     << includedSubDir));

    const QString dirPrefix = mainDir->path();

    // write the config with the exclude filters
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + includedSubDir,
                       QStringList(),
                       QStringList()
                       << excludeFilter1
                       << excludeFilter2,
                       false);

    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());

    // run through our folders that should be excluded
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + indexedRootDir));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir1));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir1));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubDir2));
    QVERIFY(!cfg->shouldFolderBeIndexed(dirPrefix + excludedSubSubDir2));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + includedSubDir));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + includedSubSubDir));

    // and some file checks
    const QString fileName = QStringLiteral("/somefile.txt");
    QVERIFY(cfg->shouldBeIndexed(dirPrefix + indexedRootDir + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubDir1 + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubSubDir1 + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubDir2 + fileName));
    QVERIFY(!cfg->shouldBeIndexed(dirPrefix + excludedSubSubDir2 + fileName));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + includedSubDir + fileName));
    QVERIFY(cfg->shouldFolderBeIndexed(dirPrefix + includedSubSubDir + fileName));
}

QTEST_GUILESS_MAIN(FileIndexerConfigTest)

#include "fileindexerconfigtest.moc"
