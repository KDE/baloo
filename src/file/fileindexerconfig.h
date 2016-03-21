/*
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef BALOO_FILEINDEXER_SERVICE_CONFIG_H_
#define BALOO_FILEINDEXER_SERVICE_CONFIG_H_

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSet>

#include <kconfig.h>

#include "regexpcache.h"

namespace Baloo
{

class StorageDevices;

/**
 * Active config class which emits signals if the config
 * was changed, for example if the KCM saved the config file.
 */
class FileIndexerConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a new file indexr config.
     */
    explicit FileIndexerConfig(QObject* parent = 0);
    ~FileIndexerConfig();

    /**
     * The folders to search for files to analyze. Cached and cleaned up.
     */
    QStringList includeFolders() const;

    /**
     * The folders that should be excluded. Cached and cleaned up.
     * It is perfectly possible to include subfolders again.
     */
    QStringList excludeFolders() const;

    QStringList excludeFilters() const;

    QStringList excludeMimetypes() const;

    bool indexHiddenFilesAndFolders() const;

    bool onlyBasicIndexing() const;

    /**
     * true the first time the service is run (or after manually
     * tampering with the config.
     */
    bool isInitialRun() const;

    /**
     * A "hidden" config option which allows to disable the initial
     * update of all indexed folders.
     *
     * This should be used in combination with isInitialRun() to make
     * sure all folders are at least indexed once.
     */
    bool initialUpdateDisabled() const;

    /**
     * Check if \p path should be indexed taking into account
     * the includeFolders(), the excludeFolders(), and the
     * excludeFilters().
     *
     * Be aware that this method does not check if parent dirs
     * match any of the exclude filters. Only the name of the
     * dir itself it checked.
     *
     * \return \p true if the file or folder at \p path should
     * be indexed according to the configuration.
     */
    bool shouldBeIndexed(const QString& path) const;

    /**
     * Check if the folder at \p path should be indexed.
     *
     * Be aware that this method does not check if parent dirs
     * match any of the exclude filters. Only the name of the
     * dir itself it checked.
     *
     * \return \p true if the folder at \p path should
     * be indexed according to the configuration.
     */
    bool shouldFolderBeIndexed(const QString& path) const;

    /**
     * Check \p fileName for all exclude filters. This does
     * not take file paths into account.
     *
     * \return \p true if a file with name \p filename should
     * be indexed according to the configuration.
     */
    bool shouldFileBeIndexed(const QString& fileName) const;

    /**
     * Checks if \p mimeType should be indexed
     *
     * \return \p true if the mimetype should be indexed according
     * to the configuration
     */
    bool shouldMimeTypeBeIndexed(const QString& mimeType) const;

    /**
     * Returns true if the folder is in the list indexed directories
     * and not in the list of exclude directories
     */
    bool folderInFolderList(const QString& path);

    /**
     * Check if \p path is in the list of folders to be indexed taking
     * include and exclude folders into account.
     * \p folder is set to the folder which was the reason for the descision.
     */
    bool folderInFolderList(const QString& path, QString& folder) const;

    /**
     * Returns the internal version number of the Baloo database
     */
    int databaseVersion() const;
    void setDatabaseVersion(int version);

    bool indexingEnabled() const;

    /**
      * Returns batch size
      */
    uint maxUncomittedFiles();

public Q_SLOTS:
    /**
     * Reread the config from disk and update the configuration cache.
     * This is only required for testing as normally the config updates
     * itself whenever the config file on disk changes.
     *
     * \return \c true if the config has actually changed
     */
    void forceConfigUpdate();

    /**
     * Should be called once the initial indexing is done, ie. all folders
     * have been indexed.
     */
    void setInitialRun(bool isInitialRun);

private:

    void buildFolderCache();
    void buildExcludeFilterRegExpCache();
    void buildMimeTypeCache();

    KConfig m_config;

    /// Caching cleaned up list (no duplicates, no useless entries, etc.)
    QList<QPair<QString, bool> > m_folderCache;

    /// cache of regexp objects for all exclude filters
    /// to prevent regexp parsing over and over
    RegExpCache m_excludeFilterRegExpCache;

    /// A set of mimetypes which should never be indexed
    QSet<QString> m_excludeMimetypes;

    bool m_indexHidden;
    bool m_onlyBasicIndexing;

    StorageDevices* m_devices;

    const uint m_maxUncomittedFiles;
};

}

#endif
