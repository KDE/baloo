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

#ifndef FILEINDEXERCONFIGUTILS_H
#define FILEINDEXERCONFIGUTILS_H

#include <KConfig>
#include <KConfigGroup>
#include <KTempDir>

#include <QDir>
#include <QTextStream>

namespace Baloo
{
namespace Test
{
void writeIndexerConfig(const QStringList& includeFolders,
                        const QStringList& excludeFolders,
                        const QStringList& excludeFilters = QStringList(),
                        bool indexHidden = false)
{
    KConfig fileIndexerConfig("baloofilerc");
    fileIndexerConfig.group("General").writePathEntry("folders", includeFolders);
    fileIndexerConfig.group("General").writePathEntry("exclude folders", excludeFolders);
    fileIndexerConfig.group("General").writeEntry("exclude filters", excludeFilters);
    fileIndexerConfig.group("General").writeEntry("index hidden folders", indexHidden);
    fileIndexerConfig.sync();
}

KTempDir* createTmpFolders(const QStringList& folders)
{
    KTempDir* tmpDir = new KTempDir();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (tmpDir->name().contains("/.")) {
        delete tmpDir;
        tmpDir = new KTempDir(QLatin1String("/tmp/"));
    }
    Q_FOREACH (const QString & f, folders) {
        QDir dir(tmpDir->name());
        Q_FOREACH (const QString & sf, f.split('/', QString::SkipEmptyParts)) {
            if (!dir.exists(sf)) {
                dir.mkdir(sf);
            }
            dir.cd(sf);
        }
    }
    return tmpDir;
}


KTempDir* createTmpFilesAndFolders(const QStringList& list)
{
    KTempDir* tmpDir = new KTempDir();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (tmpDir->name().contains("/.")) {
        delete tmpDir;
        tmpDir = new KTempDir(QLatin1String("/tmp/"));
    }
    Q_FOREACH (const QString& f, list) {
        if (f.endsWith('/')) {
            QDir dir(tmpDir->name());
            Q_FOREACH (const QString & sf, f.split('/', QString::SkipEmptyParts)) {
                if (!dir.exists(sf)) {
                    dir.mkdir(sf);
                }
                dir.cd(sf);
            }
        }
        else {
            QFile file(tmpDir->name() + f);
            file.open(QIODevice::WriteOnly);

            QTextStream stream(&file);
            stream << "test";
        }
    }
    return tmpDir;
}
//
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
const QString indexedRootDir = QString::fromLatin1("d1");
const QString indexedSubDir = QString::fromLatin1("d1/sd1");
const QString indexedSubSubDir = QString::fromLatin1("d1/sd1/ssd1");
const QString excludedSubSubDir = QString::fromLatin1("d1/sd1/ssd2");
const QString hiddenSubSubDir = QString::fromLatin1("d1/sd1/.ssd3");
const QString ignoredSubFolderToIndexedHidden = QString::fromLatin1("d1/sd1/.ssd3/isfh1");
const QString indexedSubFolderToIndexedHidden = QString::fromLatin1("d1/sd1/.ssd3/isfh2");
const QString excludedSubDir = QString::fromLatin1("d1/sd2");
const QString indexedSubDirToExcluded = QString::fromLatin1("d1/sd2/isde1");
const QString indexedHiddenSubDirToExcluded = QString::fromLatin1("d1/sd2/.isde2");
const QString hiddenSubDir = QString::fromLatin1("d1/.sd3");
const QString indexedHiddenSubDir = QString::fromLatin1("d1/.sd4");
const QString ignoredRootDir = QString::fromLatin1("d2");
const QString excludedRootDir = QString::fromLatin1("d3");
}
}

#endif // FILEINDEXERCONFIGUTILS_H
