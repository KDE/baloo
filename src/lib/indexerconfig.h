/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_INDEXERCONFIG_H
#define BALOO_INDEXERCONFIG_H

#include <QObject>
#include "core_export.h"

#include <memory>

namespace Baloo {

class FileIndexerConfigData;

class BALOO_CORE_EXPORT IndexerConfigData
{
public:
    IndexerConfigData(bool fileIndexingEnabled, bool onlyBasicIndexing, Baloo::FileIndexerConfigData configData);
    IndexerConfigData(const IndexerConfigData &);
    IndexerConfigData(IndexerConfigData &&);
    IndexerConfigData &operator=(const IndexerConfigData &);
    IndexerConfigData &operator=(IndexerConfigData &&);

    virtual ~IndexerConfigData();

    virtual bool fileIndexingEnabled() const;
    virtual bool shouldBeIndexed(const QString &path) const;
    virtual bool onlyBasicIndexing() const;
    virtual std::shared_ptr<IndexerConfigData> configData() const;

private:
    class Private;
    std::unique_ptr<Private> d;

    IndexerConfigData();
    friend class IndexerConfig;
};

/**
 * @class IndexerConfig indexerconfig.h <Baloo/IndexerConfig>
 */
class BALOO_CORE_EXPORT IndexerConfig : public IndexerConfigData
{
public:
    IndexerConfig();
    ~IndexerConfig();

    IndexerConfig(const IndexerConfig &) = delete;
    IndexerConfig &operator=(const IndexerConfig &) = delete;

    bool fileIndexingEnabled() const override;
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
    bool shouldBeIndexed(const QString &path) const override;

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

    bool indexHidden() const;
    void setIndexHidden(bool value) const;

    bool onlyBasicIndexing() const override;
    void setOnlyBasicIndexing(bool value);
    std::shared_ptr<IndexerConfigData> configData() const override;

    void refresh() const;

private:
    class Private;
    std::unique_ptr<Private> const d;
};
}

#endif // BALOO_INDEXERCONFIG_H
