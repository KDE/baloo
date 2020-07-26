/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fileindexerconfigutils.h"
#include "filtereddiriterator.h"
#include "fileindexerconfig.h"

#include <QTemporaryDir>
#include <QTest>

class FilteredDirIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testFolders();
    void testAddingExcludedFolder();
    void testNoConfig();
};

using namespace Baloo;

void FilteredDirIteratorTest::testFiles()
{
    // Given
    QStringList dirs;
    dirs << QStringLiteral("home/");
    dirs << QStringLiteral("home/1");
    dirs << QStringLiteral("home/2");
    dirs << QStringLiteral("home/kde/");
    dirs << QStringLiteral("home/kde/1");
    dirs << QStringLiteral("home/docs/");
    dirs << QStringLiteral("home/docs/.fire");
    dirs << QStringLiteral("home/docs/1");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first());

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
    }
    QSet<QString> expected = {QStringLiteral("/home"), QStringLiteral("/home/docs"), QStringLiteral("/home/docs/1"), QStringLiteral("/home/1"), QStringLiteral("/home/2")};
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testFolders()
{
    // Given
    QStringList dirs;
    dirs << QStringLiteral("home/");
    dirs << QStringLiteral("home/1");
    dirs << QStringLiteral("home/2");
    dirs << QStringLiteral("home/kde/");
    dirs << QStringLiteral("home/kde/1");
    dirs << QStringLiteral("home/docs/");
    dirs << QStringLiteral("home/docs/1");
    dirs << QStringLiteral("home/fire/");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first(), FilteredDirIterator::DirsOnly);

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
    }
    QSet<QString> expected = {QStringLiteral("/home"), QStringLiteral("/home/docs"), QStringLiteral("/home/fire")};
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testAddingExcludedFolder()
{
    // Given
    QStringList dirs;
    dirs << QStringLiteral("home/");
    dirs << QStringLiteral("home/kde/");
    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, excludeFolders.first());

    QVERIFY(it.next().isEmpty());
}

void FilteredDirIteratorTest::testNoConfig()
{
    // Given
    QStringList dirs;
    dirs << QStringLiteral("home/");
    dirs << QStringLiteral("home/1");
    dirs << QStringLiteral("home/kde/");
    dirs << QStringLiteral("home/kde/2");
    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FilteredDirIterator it(nullptr, includeFolders.first());

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
    }
    QSet<QString> expected = {QStringLiteral("/home"), QStringLiteral("/home/1"), QStringLiteral("/home/kde"), QStringLiteral("/home/kde/2")};
    QCOMPARE(list, expected);
}

QTEST_GUILESS_MAIN(FilteredDirIteratorTest)

#include "filtereddiriteratortest.moc"
