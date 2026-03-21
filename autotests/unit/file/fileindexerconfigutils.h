/*
    This file is part of the Nepomuk KDE project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FILEINDEXERCONFIGUTILS_H
#define FILEINDEXERCONFIGUTILS_H

#include <KConfig>
#include <KConfigGroup>

#include <memory>

#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>

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
    fileIndexerConfig.group(QStringLiteral("General")).writePathEntry("folders", includeFolders);
    fileIndexerConfig.group(QStringLiteral("General")).writePathEntry("exclude folders", excludeFolders);
    fileIndexerConfig.group(QStringLiteral("General")).writeEntry("exclude filters", excludeFilters);
    fileIndexerConfig.group(QStringLiteral("General")).writeEntry("index hidden folders", indexHidden);
    fileIndexerConfig.sync();
}

std::unique_ptr<QTemporaryDir> createTmpFilesAndFolders(const QStringList& list)
{
    auto tmpDir = std::make_unique<QTemporaryDir>();
    // If the temporary directory is in a hidden folder, then the tests will fail,
    // so we use /tmp/ instead.
    // TODO: Find a better solution
    if (QFileInfo(tmpDir->path()).isHidden()) {
        tmpDir = std::make_unique<QTemporaryDir>(QStringLiteral("/tmp/"));
    }
    for (const QString& f : list) {
        if (f.endsWith(QLatin1Char('/'))) {
            QDir dir(tmpDir->path());
            const auto lst = f.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            for (const QString & sf : lst) {
                if (!dir.exists(sf)) {
                    dir.mkdir(sf);
                }
#ifdef Q_OS_WIN
                if(sf.startsWith(QLatin1Char('.'))) {
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

            file.write("test");
        }
    }
    return tmpDir;
}
} // namespace Test
} // namespace Baloo

#endif // FILEINDEXERCONFIGUTILS_H
