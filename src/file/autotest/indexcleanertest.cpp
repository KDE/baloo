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

#include "indexcleanertest.h"
#include "../indexcleaner.h"
#include "../fileindexerconfig.h"
#include "fileindexerconfigutils.h"

#include <KConfig>
#include <KTempDir>
#include <KDebug>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QScopedPointer>


#include "qtest_kde.h"

using namespace Nepomuk2::Test;


void IndexCleanerTest::testConstructExcludeFolderFilter()
{
    // create the full folder hierarchy which includes some special cases:
    QScopedPointer<KTempDir> mainDir(createTmpFolders(QStringList()
                                     << indexedRootDir
                                     << indexedSubDir
                                     << indexedSubSubDir
                                     << excludedSubSubDir
                                     << hiddenSubSubDir
                                     << ignoredSubFolderToIndexedHidden
                                     << indexedSubFolderToIndexedHidden
                                     << indexedSubDirToExcluded
                                     << indexedHiddenSubDirToExcluded
                                     << excludedRootDir
                                     << hiddenSubDir
                                     << indexedHiddenSubDir
                                     << ignoredRootDir
                                     << excludedRootDir));

    const QString dirPrefix = mainDir->name();

    // write the config
    writeIndexerConfig(QStringList()
                       << dirPrefix + indexedRootDir
                       << dirPrefix + indexedSubFolderToIndexedHidden
                       << dirPrefix + indexedHiddenSubDir
                       << dirPrefix + indexedSubDirToExcluded
                       << dirPrefix + indexedHiddenSubDirToExcluded,
                       QStringList()
                       << dirPrefix + excludedRootDir
                       << dirPrefix + excludedSubDir
                       << dirPrefix + excludedSubSubDir,
                       QStringList(),
                       false);

    // create our test config object
    QScopedPointer<Nepomuk2::FileIndexerConfig> cfg(new Nepomuk2::FileIndexerConfig());

    const QString expectedFilter
        = QString::fromLatin1("FILTER("
                              "(?url!=<file://%1d1>) && "
                              "(?url!=<file://%1d1/.sd4>) && "
                              "(?url!=<file://%1d1/sd1/.ssd3/isfh2>) && "
                              "(?url!=<file://%1d1/sd2/.isde2>) && "
                              "(?url!=<file://%1d1/sd2/isde1>) && "
                              "(!REGEX(STR(?url),'^file://%1d1/') || "
                              "REGEX(STR(?url),'^file://%1d1/sd1/ssd2/') || "
                              "(REGEX(STR(?url),'^file://%1d1/sd2/') && "
                              "!REGEX(STR(?url),'^file://%1d1/sd2/.isde2/') && "
                              "!REGEX(STR(?url),'^file://%1d1/sd2/isde1/')))) .")
          .arg(dirPrefix);
    QCOMPARE(Nepomuk2::IndexCleaner::constructExcludeFolderFilter(cfg.data()),
             expectedFilter);
}

// simplest case: one folder which should be excluded via filters
void IndexCleanerTest::testConstructExcludeFiltersFolderFilter1()
{
    QScopedPointer<KTempDir> mainDir(createTmpFolders(QStringList()
                                     << QLatin1String("root/x_y/sub")));

    const QString dirPrefix = mainDir->name();

    // write the config
    writeIndexerConfig(QStringList()
                       << dirPrefix + QLatin1String("root"),
                       QStringList(),
                       QStringList()
                       << QLatin1String("x_y"),
                       false);

    // create our test config object
    QScopedPointer<Nepomuk2::FileIndexerConfig> cfg(new Nepomuk2::FileIndexerConfig());

    QString expectedFilter = QLatin1String("((REGEX(STR(?url),'/x_y/')))");
    QCOMPARE(Nepomuk2::IndexCleaner::constructExcludeFiltersFolderFilter(cfg.data()),
             expectedFilter);


    // second simple case: include the sub dir again
    writeIndexerConfig(QStringList()
                       << dirPrefix + QLatin1String("root")
                       << dirPrefix + QLatin1String("root/x_y/sub"),
                       QStringList(),
                       QStringList()
                       << QLatin1String("x_y"),
                       false);
    cfg->forceConfigUpdate();

    expectedFilter = QString::fromLatin1("(?url!=<file://%1root/x_y/sub>) && ((REGEX(STR(?url),'/x_y/') && (!REGEX(STR(?url),'^file://%1root/x_y/sub/') || REGEX(bif:substring(STR(?url),%2,10000),'/x_y/'))))")
                     .arg(dirPrefix)
                     .arg(dirPrefix.length() + 7 + 13);
    QCOMPARE(Nepomuk2::IndexCleaner::constructExcludeFiltersFolderFilter(cfg.data()),
             expectedFilter);
}

// test with two sub-dirs and two filters
void IndexCleanerTest::testConstructExcludeFiltersFolderFilter2()
{
    QScopedPointer<KTempDir> mainDir(createTmpFolders(QStringList()
                                     << QLatin1String("root/x_y/sub1/x_z/sub2")));

    const QString dirPrefix = mainDir->name();

    // write the config
    writeIndexerConfig(QStringList()
                       << dirPrefix + QLatin1String("root")
                       << dirPrefix + QLatin1String("root/x_y/sub1")
                       << dirPrefix + QLatin1String("root/x_y/sub1/x_z/sub2"),
                       QStringList(),
                       QStringList()
                       << QLatin1String("x_y")
                       << QLatin1String("x_z"),
                       false);

    // create our test config object
    QScopedPointer<Nepomuk2::FileIndexerConfig> cfg(new Nepomuk2::FileIndexerConfig());

    QString expectedFilter = QString::fromLatin1("(?url!=<file://%1root/x_y/sub1/x_z/sub2>) && (?url!=<file://%1root/x_y/sub1>) && "
                             "("
                             "(REGEX(STR(?url),'/x_y/') && "
                             "(!REGEX(STR(?url),'^file://%1root/x_y/sub1/x_z/sub2/') || REGEX(bif:substring(STR(?url),%2,10000),'/x_y/')) && "
                             "(!REGEX(STR(?url),'^file://%1root/x_y/sub1/') || REGEX(bif:substring(STR(?url),%3,10000),'/x_y/'))"
                             ") || "
                             "(REGEX(STR(?url),'/x_z/') && "
                             "(!REGEX(STR(?url),'^file://%1root/x_y/sub1/x_z/sub2/') || REGEX(bif:substring(STR(?url),%2,10000),'/x_z/'))"
                             ")"
                             ")")
                             .arg(dirPrefix)
                             .arg(dirPrefix.length() + 7 + 23)
                             .arg(dirPrefix.length() + 7 + 14);
    QCOMPARE(Nepomuk2::IndexCleaner::constructExcludeFiltersFolderFilter(cfg.data()),
             expectedFilter);
}

QTEST_KDEMAIN_CORE(IndexCleanerTest)

#include "indexcleanertest.moc"
