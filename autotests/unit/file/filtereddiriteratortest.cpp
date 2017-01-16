/*
   This file is part of the KDE Baloo project.
   Copyright (C) 2014 Vishesh Handa <vhanda@kde.org>

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
