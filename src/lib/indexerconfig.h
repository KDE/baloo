/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_INDEXERCONFIG_H
#define BALOO_INDEXERCONFIG_H

#include <QObject>
#include "core_export.h"

namespace Baloo {

/**
 * @class IndexerConfig indexerconfig.h <Baloo/IndexerConfig>
 *
 * This class allows it to access the current config, to
 * read and modify it.
 *
 * It is not meant as a means to infer the current state of
 * the Indexer or the DB. The DB is updated asynchronously,
 * and changes of the config will not be reflected immediately.
 */
class BALOO_CORE_EXPORT IndexerConfig
{
public:
    IndexerConfig();
    ~IndexerConfig();

    IndexerConfig(const IndexerConfig &) = delete;
    IndexerConfig &operator=(const IndexerConfig &) = delete;

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
    *
    * TODO KF6: deprecate, not of any concern for ouside
    * users. Use \c Baloo::File to know if a file has
    * been indexed.
    * \sa Baloo::File
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

#if BALOO_CORE_BUILD_DEPRECATED_SINCE(5, 69)
    /**
     * @deprecated Since 5.69. Do not use this function, firstRun is no longer
     * exposed in the config, as it is an internal baloo state.
     * \return \c false
     */
    BALOO_CORE_DEPRECATED_VERSION(5, 69, "Do not use. firstRun is a baloo internal state")
    bool firstRun() const;
    /**
     * @deprecated Since 5.69. Do not use this function. Since 5.69, calling
     * it no longer has any effect.
     */
    BALOO_CORE_DEPRECATED_VERSION(5, 69, "Do not use. firstRun is a baloo internal state")
    void setFirstRun(bool firstRun) const;
#endif

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
