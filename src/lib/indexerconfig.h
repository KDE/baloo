/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <vhanda@kde.org>
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

#ifndef BALOO_INDEXERCONFIG_H
#define BALOO_INDEXERCONFIG_H

#include <QObject>
#include "core_export.h"

namespace Baloo {

/**
 * @class IndexerConfig indexerconfig.h <Baloo/IndexerConfig>
 */
class BALOO_CORE_EXPORT IndexerConfig
{
public:
    IndexerConfig();
    ~IndexerConfig();

    bool fileIndexingEnabled() const;
    void setFileIndexingEnabled(bool enabled) const;

    /**
    * Check if the file or folder \p path should be indexed.
    * 
    * If itself or its nearest explicitly included or excluded ancestor is 
    * excluded it is not indexed. 
    * Otherwise it is indexed according to the 
    * includeFolders and excludeFilters config.
    *
    * \return \c true if the file or folder at \p path should
    * be indexed according to the configuration.
    */
    bool shouldBeIndexed(const QString& path) const;

    /**
    * Check if \p folder can be searched.
    * \p folder can be searched if itself or one of its descendants is indexed.
    * 
    * Example:
    * if ~/foo is not indexed and ~/foo/bar is indexed
    * then ~/foo can be searched.
    *
    * \return \c true if the \p folder can be searched.
    */
    bool canBeSearched(const QString& folder) const;
    
    /**
     * Folders to search for files to index and analyze.
     * \return list of paths.
     */
    QStringList includeFolders() const;
    
    /**
     * Folders that are excluded from indexing.
     * (Descendant folders of an excluded folder can be added 
     * and they will be indexed.)
     * \return list of paths.
     */
    QStringList excludeFolders() const;
    QStringList excludeFilters() const;
    QStringList excludeMimetypes() const;

    void setIncludeFolders(const QStringList& includeFolders);
    void setExcludeFolders(const QStringList& excludeFolders);
    void setExcludeFilters(const QStringList& excludeFilters);
    void setExcludeMimetypes(const QStringList& excludeMimetypes);

    /**
     * \return \c true if the service is run for the first time 
     * (or after manually setting "first run=true" in the config).
     */
    bool firstRun() const;
    void setFirstRun(bool firstRun) const;

    bool indexHidden() const;
    void setIndexHidden(bool value) const;

    bool onlyBasicIndexing() const;
    void setOnlyBasicIndexing(bool value);

    void refresh() const;

private:
    class Private;
    Private* d;
};
}

#endif // BALOO_INDEXERCONFIG_H
