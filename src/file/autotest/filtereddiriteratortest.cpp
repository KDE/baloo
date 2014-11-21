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
#include "../filtereddiriterator.h"
#include "../fileindexerconfig.h"

#include <QTemporaryDir>
#include <QTest>
#include <QDebug>

class FilteredDirIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testFolders();
};

using namespace Baloo;

void FilteredDirIteratorTest::testFiles()
{
    // Given
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

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first(), FilteredDirIterator::Files);

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
    }
    QSet<QString> expected = {"/home", "/home/docs", "/home/docs/1", "/home/1", "/home/2"};
    QCOMPARE(list, expected);
}

void FilteredDirIteratorTest::testFolders()
{
    // Given
    QStringList dirs;
    dirs << QLatin1String("home/");
    dirs << QLatin1String("home/1");
    dirs << QLatin1String("home/2");
    dirs << QLatin1String("home/kde/");
    dirs << QLatin1String("home/kde/1");
    dirs << QLatin1String("home/docs/");
    dirs << QLatin1String("home/docs/1");
    dirs << QLatin1String("home/fire/");

    QScopedPointer<QTemporaryDir> dir(Test::createTmpFilesAndFolders(dirs));

    QStringList includeFolders;
    includeFolders << dir->path() + QLatin1String("/home");

    QStringList excludeFolders;
    excludeFolders << dir->path() + QLatin1String("/home/kde");

    Test::writeIndexerConfig(includeFolders, excludeFolders);

    FileIndexerConfig config;
    FilteredDirIterator it(&config, includeFolders.first(), FilteredDirIterator::Dirs);

    QSet<QString> list;
    while (!it.next().isEmpty()) {
        QString path = it.filePath();
        path = path.mid(dir->path().length());

        list << path;
    }
    QSet<QString> expected = {"/home", "/home/docs", "/home/fire"};
    QCOMPARE(list, expected);
}


QTEST_MAIN(FilteredDirIteratorTest)

#include "filtereddiriteratortest.moc"
