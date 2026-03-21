/*
    This file is part of the Baloo KDE project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fileindexerconfig.h"
#include "fileindexerconfigutils.h"

#include <memory>
#include <QObject>
#include <QFile>
#include <QTest>
#include <QTemporaryDir>

namespace Baloo {
namespace Test {

using namespace Qt::Literals::StringLiterals;

class FileIndexerConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void testShouldFolderBeIndexed();
    void testShouldFolderBeIndexed_data();

    void testShouldFolderBeIndexedHidden();
    void testShouldFolderBeIndexedHidden_data();

    void testShouldBeIndexed();
    void testShouldBeIndexed_data();

    void testShouldBeIndexedHidden();
    void testShouldBeIndexedHidden_data();

    void testShouldExcludeFilterOnFolders();
    void testShouldExcludeFilterOnFolders_data();

private:
    std::unique_ptr<QTemporaryDir> m_mainDir;
    QString m_dirPrefix;
};

namespace
{
// Trying to put all cases into one folder tree:
// |- indexedRootDir
//   |- indexedSubDir
//     |- indexedSubSubDir
//     |- excludedSubSubDir
//     |- .hiddenSubSubDir
//       |- ignoredSubFolderToIndexedHidden
//       |- indexedSubFolderToIndexedHidden
//   |- excludedSubDir
//     |- indexedSubDirToExcluded
//     |- .indexedHiddenSubDirToExcluded
//   |- .hiddenSubDir
//   |- .indexedHiddenSubDir
// |- ignoredRootDir
// |- excludedRootDir
//
const QString indexedRootDir = u"d1/"_s;
const QString indexedSubDir = u"d1/sd1/"_s;
const QString indexedSubSubDir = u"d1/sd1/ssd1/"_s;
const QString excludedSubSubDir = u"d1/sd1/ssd2/"_s;
const QString hiddenSubSubDir = u"d1/sd1/.ssd3/"_s;
const QString ignoredSubFolderToIndexedHidden = u"d1/sd1/.ssd3/isfh1/"_s;
const QString indexedSubFolderToIndexedHidden = u"d1/sd1/.ssd3/isfh2/"_s;
const QString excludedSubDir = u"d1/sd2/"_s;
const QString indexedSubDirToExcluded = u"d1/sd2/isde1/"_s;
const QString indexedHiddenSubDirToExcluded = u"d1/sd2/.isde2/"_s;
const QString hiddenSubDir = u"d1/.sd3/"_s;
const QString indexedHiddenSubDir = u"d1/.sd4/"_s;
const QString ignoredRootDir = u"d2/"_s;
const QString excludedRootDir = u"d3/"_s;

const QString excludeFilter1 = u"build"_s;
const QString excludeFilter2 = u"foo?ar"_s;

const QString excludedFilterSubDir1 = indexedRootDir + u"build/"_s;
const QString excludedFilterSubSubDir1 = indexedRootDir + u"build/sub1/"_s;

const QString excludedFilterSubDir2 = indexedRootDir + u"foobar/"_s;
const QString excludedFilterSubSubDir2 = indexedRootDir + u"foobar/sub2/"_s;

const QString includedFilterSubDir = indexedRootDir + u"build/sub3/"_s;
const QString includedFilterSubSubDir = indexedRootDir + u"build/sub3/sub/";
}

void FileIndexerConfigTest::initTestCase()
{
    m_mainDir = createTmpFilesAndFolders(QStringList{
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
        includedFilterSubSubDir,
    });
    m_dirPrefix = m_mainDir->path() + QLatin1Char('/');
}

void FileIndexerConfigTest::cleanupTestCase()
{
    m_mainDir.reset();
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
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << true;
    }

    for (const auto& key : excluded) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << false;
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
    Baloo::FileIndexerConfig cfg;
    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);

    QCOMPARE(cfg.shouldFolderBeIndexed(path), shouldBeIndexed);
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
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << true;
    }

    for (const auto& key : excluded) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << false;
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
    Baloo::FileIndexerConfig cfg;
    cfg.forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);

    QCOMPARE(cfg.shouldFolderBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldBeIndexed_data()
{
    const QString fileName = u"somefile.txt"_s;
    const auto indexed = QStringList{
        indexedRootDir,
        indexedSubDir,
        indexedSubSubDir,
        indexedSubFolderToIndexedHidden,
        indexedSubDirToExcluded,
        indexedHiddenSubDir,
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

    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");

    for (const auto& key : indexed) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << true;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << true;
    }

    for (const auto& key : excluded) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << false;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << false;
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
    Baloo::FileIndexerConfig cfg;
    cfg.forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);

    QCOMPARE(cfg.shouldBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldBeIndexedHidden_data()
{
    const QString fileName = u"somefile.txt"_s;

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
        excludedRootDir,
    };

    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");

    for (const auto& key : indexed) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(u"Not a folder: %1"_s.arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << dirName << true;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << true;
    }

    for (const auto& key : excluded) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(m_dirPrefix + key).isDir(), qPrintable(u"Not a folder: %1"_s.arg(m_dirPrefix + key)));
        QTest::addRow("%s", qPrintable(key)) << dirName << false;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << false;
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
    Baloo::FileIndexerConfig cfg;
    cfg.forceConfigUpdate();

    QFETCH(QString, path);
    QFETCH(bool, shouldBeIndexed);

    QCOMPARE(cfg.shouldBeIndexed(path), shouldBeIndexed);
}

void FileIndexerConfigTest::testShouldExcludeFilterOnFolders_data()
{
    const QString fileName = u"somefile.txt"_s;
    const auto indexed = QStringList{
        indexedRootDir,
        includedFilterSubDir,
        includedFilterSubSubDir,
    };
    const auto excluded = QStringList{
        excludedFilterSubDir1,
        excludedFilterSubSubDir1,
        excludedFilterSubDir2,
        excludedFilterSubSubDir2,
    };

    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("shouldBeIndexed");

    for (const auto& key : indexed) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << true;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << true;
    }

    for (const auto& key : excluded) {
        const auto dirName = m_dirPrefix + key;
        QVERIFY2(QFileInfo(dirName).isDir(), qPrintable(u"Not a folder: %1"_s.arg(dirName)));
        QTest::addRow("%s", qPrintable(key)) << dirName << false;
        QTest::addRow("%s - file", qPrintable(key)) << dirName + fileName << false;
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

    Baloo::FileIndexerConfig cfg;
    cfg.forceConfigUpdate();                                //Maybe this was left out on purpose

    QCOMPARE(cfg.shouldFolderBeIndexed(path), shouldBeIndexed);
}

} // namespace Test
} // namespace Baloo

using namespace Baloo::Test;

QTEST_GUILESS_MAIN(FileIndexerConfigTest)

#include "fileindexerconfigtest.moc"
