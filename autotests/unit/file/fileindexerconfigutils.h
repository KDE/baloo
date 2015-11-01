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

#include <QDir>
#include <QTextStream>
#include <QTemporaryDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Baloo
{
namespace Test
{
void writeIndexerConfig(const QStringList& includeFolders,
                        const QStringList& excludeFolders,
                        const QStringList& excludeFilters = QStringList(),
                        bool indexHidden = false)
{
    QStandardPaths::setTestModeEnabled(true);
    KConfig fileIndexerConfig(QStringLiteral("baloofilerc"));
    fileIndexerConfig.group("General").writePathEntry("folders", includeFolders);
    fileIndexerConfig.group("General").writePathEntry("exclude folders", excludeFolders);
    fileIndexerConfig.group("General").writeEntry("exclude filters", excludeFilters);
    fileIndexerConfig.group("General").writeEntry("index hidden folders", indexHidden);
    fileIndexerConfig.sync();
}

QTemporaryDir* createTmpFolders(const QStringList& folders)
{
    QTemporaryDir* tmpDir = new QTemporaryDir();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (QFileInfo(tmpDir->path()).isHidden()) {
        delete tmpDir;
        tmpDir = new QTemporaryDir(QStringLiteral("/tmp/"));
    }
    Q_FOREACH (const QString & f, folders) {
        QDir dir(tmpDir->path());
        Q_FOREACH (const QString & sf, f.split(QLatin1Char('/'), QString::SkipEmptyParts)) {
            if (!dir.exists(sf)) {
                dir.mkdir(sf);
            }
#ifdef Q_OS_WIN
            if(sf.startsWith(QLatin1String("."))) {
                if(!SetFileAttributesW(reinterpret_cast<const WCHAR*>((dir.path() + "/" + sf).utf16()), FILE_ATTRIBUTE_HIDDEN)) {
                    qWarning("failed to set 'hidden' attribute!");
                }
            }
#endif
            dir.cd(sf);
        }
    }
    return tmpDir;
}


QTemporaryDir* createTmpFilesAndFolders(const QStringList& list)
{
    QTemporaryDir* tmpDir = new QTemporaryDir();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (QFileInfo(tmpDir->path()).isHidden()) {
        delete tmpDir;
        tmpDir = new QTemporaryDir(QStringLiteral("/tmp/"));
    }
    Q_FOREACH (const QString& f, list) {
        if (f.endsWith(QLatin1Char('/'))) {
            QDir dir(tmpDir->path());
            Q_FOREACH (const QString & sf, f.split(QLatin1Char('/'), QString::SkipEmptyParts)) {
                if (!dir.exists(sf)) {
                    dir.mkdir(sf);
                }
#ifdef Q_OS_WIN
                if(sf.startsWith(QLatin1String("."))) {
                    if(!SetFileAttributesW(reinterpret_cast<const WCHAR*>((dir.path() + "/" + sf).utf16()), FILE_ATTRIBUTE_HIDDEN)) {
                        qWarning("failed to set 'hidden' attribute!");
                    }
                }
#endif
                dir.cd(sf);
            }
        }
        else {
            QFile file(tmpDir->path() + QLatin1Char('/') + f);
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
const QString indexedRootDir = QStringLiteral("d1");
const QString indexedSubDir = QStringLiteral("d1/sd1");
const QString indexedSubSubDir = QStringLiteral("d1/sd1/ssd1");
const QString excludedSubSubDir = QStringLiteral("d1/sd1/ssd2");
const QString hiddenSubSubDir = QStringLiteral("d1/sd1/.ssd3");
const QString ignoredSubFolderToIndexedHidden = QStringLiteral("d1/sd1/.ssd3/isfh1");
const QString indexedSubFolderToIndexedHidden = QStringLiteral("d1/sd1/.ssd3/isfh2");
const QString excludedSubDir = QStringLiteral("d1/sd2");
const QString indexedSubDirToExcluded = QStringLiteral("d1/sd2/isde1");
const QString indexedHiddenSubDirToExcluded = QStringLiteral("d1/sd2/.isde2");
const QString hiddenSubDir = QStringLiteral("d1/.sd3");
const QString indexedHiddenSubDir = QStringLiteral("d1/.sd4");
const QString ignoredRootDir = QStringLiteral("d2");
const QString excludedRootDir = QStringLiteral("d3");
}
}

#endif // FILEINDEXERCONFIGUTILS_H
