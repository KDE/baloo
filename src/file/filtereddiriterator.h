/*
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef FILTEREDDIRITERATOR_H
#define FILTEREDDIRITERATOR_H

#include <QDirIterator>
#include <QFileInfo>
#include <QStack>

namespace Baloo {

class FileIndexerConfig;

class FilteredDirIterator
{
public:
    enum Filter {
        FilesAndDirs,
        DirsOnly
    };
    FilteredDirIterator(const FileIndexerConfig* config, const QString& folder, Filter filter = FilesAndDirs);
    ~FilteredDirIterator();

    QString next();
    QString filePath() const;
    QFileInfo fileInfo() const;

private:
    /**
     * Checks if the folder should be indexed. It only performs filename checks
     * on the filename, not on every part of the path.
     */
    bool shouldIndexFolder(const QString& filePath) const;

    const FileIndexerConfig* m_config;

    QDirIterator* m_currentIter;
    QStack<QString> m_paths;
    QDir::Filters m_filters;

    QString m_filePath;
    bool m_firstItem;
};

}

#endif // FILTEREDDIRITERATOR_H
