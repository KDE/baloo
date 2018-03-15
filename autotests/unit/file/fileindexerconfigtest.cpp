/*
   This file is part of the Baloo KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2018 Michael Heidelbach <ottwolt@gmail.com>

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

#include "fileindexerconfigtest.h"
#include "fileindexerconfig.h"
#include "fileindexerconfigutils.h"

#include <QTemporaryDir>

#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QTest>


using namespace Baloo::Test;

namespace 
{
const QString excludeFilter1 = QStringLiteral("build");
const QString excludeFilter2 = QStringLiteral("foo?ar");

const QString excludedFilterSubDir1 = indexedRootDir + QLatin1String("/") + excludeFilter1;
const QString excludedFilterSubSubDir1 = excludedFilterSubDir1 + QLatin1String("/sub1");

const QString excludedFilterSubDir2 = indexedRootDir + QLatin1String("/foobar");
const QString excludedFilterSubSubDir2 = excludedFilterSubDir2 + QLatin1String("/sub2");

const QString includedFilterSubDir = excludedFilterSubDir1 + QLatin1String("/sub3");
const QString includedFilterSubSubDir = includedFilterSubDir + QLatin1String("/sub");
}

void FileIndexerConfigTest::initTestCase()
{

     m_mainDir = createTmpFolders(QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        excludedSubDir,
        excludedSubSubDir,
        hiddenSubSubDir,
        ignoredSubFolderToIndexedHidden,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        indexedHiddenSubDirToExcluded,
        excludedRootDir,
        hiddenSubDir,
        indexedHiddenSubDir,
        ignoredRootDir,
        
        excludedFilterSubDir1,
        excludedFilterSubSubDir1,
        excludedFilterSubDir2,
        excludedFilterSubSubDir2,
        includedFilterSubDir,
        includedFilterSubSubDir
     });
     m_dirPrefix = m_mainDir->path() + QLatin1Char('/');
}

void FileIndexerConfigTest::cleanupTestCase()
{
    delete m_mainDir;
}

void FileIndexerConfigTest::testShouldFolderBeIndexed_data()
{
    const auto indexed = QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        indexedHiddenSubDir
    };
    
    const auto excluded = QStringList{
        excludedSubSubDir,
        hiddenSubSubDir,
        ignoredSubFolderToIndexedHidden,
        excludedSubDir,
        hiddenSubDir,
        ignoredRootDir,
        excludedRootDir
    };
 
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");
    
    for (const auto& key : indexed) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : excluded) {
       QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }
}

void FileIndexerConfigTest::testShouldFolderBeIndexed()
{
    writeIndexerConfig(QStringList{
            m_dirPrefix + indexedRootDir,
            m_dirPrefix + indexedSubFolderToIndexedHidden,
            m_dirPrefix + indexedHiddenSubDir,
            m_dirPrefix + indexedSubDirToExcluded
        },
        QStringList{
            m_dirPrefix + excludedRootDir,
            m_dirPrefix + excludedSubDir,
            m_dirPrefix + excludedSubSubDir
        },
        QStringList(),
        false);

    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());
    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);
    
    QCOMPARE(cfg->shouldFolderBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldFolderBeIndexedHidden_data()
{

    const auto indexed = QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        hiddenSubSubDir,
        ignoredSubFolderToIndexedHidden,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        hiddenSubDir,
        indexedHiddenSubDir
    };
    const auto excluded = QStringList{
        excludedSubSubDir,
        excludedSubDir,
        ignoredRootDir,
        excludedRootDir
    };
    
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");
    
    for (const auto& key : indexed) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : excluded) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }
}

void FileIndexerConfigTest::testShouldFolderBeIndexedHidden()
{
    writeIndexerConfig(QStringList{
            m_dirPrefix + indexedRootDir,
            m_dirPrefix + indexedSubFolderToIndexedHidden,
            m_dirPrefix + indexedHiddenSubDir,
            m_dirPrefix + indexedSubDirToExcluded
        },          
        QStringList{
            m_dirPrefix + excludedRootDir,
            m_dirPrefix + excludedSubDir,
            m_dirPrefix + excludedSubSubDir
        },
        QStringList(),
        true);
    
    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());
    cfg->forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);
    
    QCOMPARE(cfg->shouldFolderBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldBeIndexed_data()
{
    const QString fileName = QStringLiteral("/somefile.txt");
    const auto indexed = QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        indexedHiddenSubDir,
    };
    const auto indexedFilenames = QStringList{
        indexedRootDir + fileName,
        indexedSubDir + fileName,
        indexedSubSubDir + fileName,
        indexedSubFolderToIndexedHidden + fileName,
        indexedSubDirToExcluded + fileName,
        indexedHiddenSubDir + fileName
    };
    const auto excluded = QStringList{
        excludedSubSubDir,
        hiddenSubSubDir,
        ignoredSubFolderToIndexedHidden,
        excludedSubDir,
        hiddenSubDir,
        ignoredRootDir,
        excludedRootDir,
    };
    const auto excludedFilenames = QStringList{
        excludedSubSubDir + fileName,
        hiddenSubSubDir + fileName,
        ignoredSubFolderToIndexedHidden + fileName,
        excludedSubDir + fileName,
        hiddenSubDir + fileName,
        ignoredRootDir + fileName,
        excludedRootDir + fileName,
    };
    
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");
    
    for (const auto& key : indexed) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : indexedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : excluded) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }
    
    for (const auto& key : excludedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }
}

void FileIndexerConfigTest::testShouldBeIndexed()
{
    writeIndexerConfig(QStringList{
            m_dirPrefix + indexedRootDir,
            m_dirPrefix + indexedSubFolderToIndexedHidden,
            m_dirPrefix + indexedHiddenSubDir,
            m_dirPrefix + indexedSubDirToExcluded
        },
        QStringList{
            m_dirPrefix + excludedRootDir,
            m_dirPrefix + excludedSubDir,
            m_dirPrefix + excludedSubSubDir
        },
        QStringList(),
        false);

    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());
    cfg->forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);

    QCOMPARE(cfg->shouldBeIndexed(path), shouldBeIndexed);

}

void FileIndexerConfigTest::testShouldBeIndexedHidden_data()
{
    const QString fileName = QStringLiteral("/somefile.txt");
    const auto indexed = QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        hiddenSubSubDir,
        ignoredSubFolderToIndexedHidden,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        hiddenSubDir,
        indexedHiddenSubDir
    };
    const auto indexedFilenames = QStringList{
        indexedRootDir + fileName,
        indexedSubDir + fileName,
        indexedSubSubDir + fileName,
        hiddenSubSubDir + fileName,
        ignoredSubFolderToIndexedHidden + fileName,
        indexedSubFolderToIndexedHidden + fileName,
        indexedSubDirToExcluded + fileName,
        hiddenSubDir + fileName,
        indexedHiddenSubDir + fileName
    };
    const auto excluded = QStringList{
        excludedSubSubDir,
        excludedSubDir,
        ignoredRootDir,
        excludedRootDir
    };
    const auto excludedFilenames = QStringList{
        excludedSubSubDir + fileName,
        excludedSubDir + fileName,
        ignoredRootDir + fileName,
        excludedRootDir + fileName
    };
    
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");
    
    for (const auto& key : indexed) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : indexedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : excluded) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }

    for (const auto& key : excludedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }
}

void FileIndexerConfigTest::testShouldBeIndexedHidden()
{
    writeIndexerConfig(QStringList{
            m_dirPrefix + indexedRootDir,
            m_dirPrefix + indexedSubFolderToIndexedHidden,
            m_dirPrefix + indexedHiddenSubDir,
            m_dirPrefix + indexedSubDirToExcluded
        },
        QStringList{
            m_dirPrefix + excludedRootDir,
            m_dirPrefix + excludedSubDir,
            m_dirPrefix + excludedSubSubDir
        },
        QStringList(),
        true);
    
    // create our test config object
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());
    cfg->forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);
    
    QCOMPARE(cfg->shouldBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldExcludeFilterOnFolders_data()
{
    const QString fileName = QStringLiteral("/somefile.txt");
    const auto indexed = QStringList{
        indexedRootDir,
        includedFilterSubDir,
        includedFilterSubSubDir,
    };
    const auto indexedFilenames = QStringList{
        indexedRootDir + fileName,
        includedFilterSubDir + fileName,
        includedFilterSubSubDir + fileName
    };
    const auto excluded = QStringList{
        excludedFilterSubDir1,
        excludedFilterSubSubDir1,
        excludedFilterSubDir2,
        excludedFilterSubSubDir2,
    };
    const auto excludedFilenames = QStringList{
        excludedFilterSubDir1 + fileName,
        excludedFilterSubSubDir1 + fileName,
        excludedFilterSubDir2 + fileName,
        excludedFilterSubSubDir2 + fileName
    };
    
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");
    
    for (const auto& key : indexed) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : indexedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << true;
    }
    
    for (const auto& key : excluded) {
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(QStringLiteral("Not a folder: %1").arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }    

    for (const auto& key : excludedFilenames) {
        QTest::addRow("%s", qPrintable(key)) << m_dirPrefix + key << false;
    }    
}

void FileIndexerConfigTest::testShouldExcludeFilterOnFolders()
{
    writeIndexerConfig(QStringList{
            m_dirPrefix + indexedRootDir,
            m_dirPrefix + includedFilterSubDir
        },
        QStringList(),
        QStringList{
            excludeFilter1,
            excludeFilter2
        },
        false);

    // create our test config object
    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);
    
    QScopedPointer<Baloo::FileIndexerConfig> cfg(new Baloo::FileIndexerConfig());
    cfg->forceConfigUpdate(); //Maybe this was left out on purpose

    QCOMPARE(cfg->shouldFolderBeIndexed(path), shouldBeIndexed);
}

QTEST_GUILESS_MAIN(FileIndexerConfigTest)
