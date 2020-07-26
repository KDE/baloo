/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "fileindexerconfig.h"
#include "fileexcludefilters.h"
#include "storagedevices.h"
#include "baloodebug.h"

#include <QStringList>
#include <QDir>

#include <QStandardPaths>
#include "baloosettings.h"

namespace
{
/// recursively check if a folder is hidden
bool isDirHidden(const QDir& dir)
{
#ifdef __unix__
    return dir.absolutePath().contains(QLatin1String("/."));
#else
    QDir d = dir;
    do {
        if (QFileInfo(d.path()).isHidden())
            return true;
    } while (d.cdUp());

    return false;
#endif
}

QString normalizeTrailingSlashes(QString&& path)
{
    while (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    path += QLatin1Char('/');
    return path;
}

}

namespace Baloo
{

FileIndexerConfig::FileIndexerConfig(QObject* parent)
    : QObject(parent)
    , m_settings(new BalooSettings(this))
    , m_folderCacheDirty(true)
    , m_indexHidden(false)
    , m_devices(nullptr)
    , m_maxUncomittedFiles(40)
{
    forceConfigUpdate();
}


FileIndexerConfig::~FileIndexerConfig()
{
}

QDebug operator<<(QDebug dbg, const FileIndexerConfig::FolderConfig& entry)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << entry.path << ": "
                  << (entry.isIncluded ? "included" : "excluded");
    return dbg;
}

QStringList FileIndexerConfig::includeFolders() const
{
    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    QStringList fl;
    for (const auto& entry : m_folderCache) {
        if (entry.isIncluded)
            fl << entry.path;
    }
    return fl;
}

QStringList FileIndexerConfig::excludeFolders() const
{
    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    QStringList fl;
    for (const auto& entry : m_folderCache) {
        if (!entry.isIncluded)
            fl << entry.path;
    }
    return fl;
}

QStringList FileIndexerConfig::excludeFilters() const
{
    // read configured exclude filters
    QStringList filters = m_settings->excludedFilters();

    // make sure we always keep the latest default exclude filters
    // TODO: there is one problem here. What if the user removed some of the default filters?
    if (m_settings->excludedFiltersVersion() < defaultExcludeFilterListVersion()) {
        filters += defaultExcludeFilterList();
        // in case the cfg entry was empty and filters == defaultExcludeFilterList()
        filters.removeDuplicates();

        // write the config directly since the KCM does not have support for the version yet
        m_settings->setExcludedFilters(filters);
        m_settings->setExcludedFiltersVersion(defaultExcludeFilterListVersion());
    }

    return filters;
}

QStringList FileIndexerConfig::excludeMimetypes() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return QStringList::fromSet(m_excludeMimetypes);
#else
    return QList<QString>(m_excludeMimetypes.begin(), m_excludeMimetypes.end());
#endif
}

bool FileIndexerConfig::indexHiddenFilesAndFolders() const
{
    return m_indexHidden;
}

bool FileIndexerConfig::onlyBasicIndexing() const
{
    return m_onlyBasicIndexing;
}

bool FileIndexerConfig::canBeSearched(const QString& folder) const
{
    QFileInfo fi(folder);
    QString path = fi.absolutePath();
    if (!fi.isDir()) {
        return false;
    } else if (shouldFolderBeIndexed(path)) {
        return true;
    }

    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    // Look for included descendants
    for (const auto& entry : m_folderCache) {
        if (entry.isIncluded && entry.path.startsWith(path)) {
            return true;
        }
    }

    return false;
}

bool FileIndexerConfig::shouldBeIndexed(const QString& path) const
{
    QFileInfo fi(path);
    if (fi.isDir()) {
        return shouldFolderBeIndexed(path);
    } else {
        return (shouldFolderBeIndexed(fi.absolutePath()) &&
                (!fi.isHidden() || indexHiddenFilesAndFolders()) &&
                shouldFileBeIndexed(fi.fileName()));
    }
}


bool FileIndexerConfig::shouldFolderBeIndexed(const QString& path) const
{
    QString folder;
    if (folderInFolderList(path, folder)) {
        // we always index the folders in the list
        // ignoring the name filters
        if (folder == normalizeTrailingSlashes(QString(path)))
            return true;

        // check for hidden folders
        QDir dir(path);
        if (!indexHiddenFilesAndFolders() && isDirHidden(dir))
            return false;

        // check the exclude filters for all components of the path
        // after folder
        const QStringList pathComponents = path.mid(folder.count()).split(QLatin1Char('/'), QString::SkipEmptyParts);
        for (const QString& c : pathComponents) {
            if (!shouldFileBeIndexed(c)) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}


bool FileIndexerConfig::shouldFileBeIndexed(const QString& fileName) const
{
    if (!indexHiddenFilesAndFolders() && fileName.startsWith(QLatin1Char('.'))) {
        return false;
    }
    return !m_excludeFilterRegExpCache.exactMatch(fileName);
}

bool FileIndexerConfig::shouldMimeTypeBeIndexed(const QString& mimeType) const
{
    return !m_excludeMimetypes.contains(mimeType);
}


bool FileIndexerConfig::folderInFolderList(const QString& path)
{
    QString str;
    return folderInFolderList(path, str);
}

bool FileIndexerConfig::folderInFolderList(const QString& path, QString& folder) const
{
    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    const QString p = normalizeTrailingSlashes(QString(path));

    for (const auto& entry : m_folderCache) {
        const QString& f = entry.path;
        if (p.startsWith(f)) {
            folder = f;
            return entry.isIncluded;
        }
    }
    // path is not in the list, thus it should not be included
    folder.clear();
    return false;
}


void FileIndexerConfig::FolderCache::cleanup()
{
    // TODO There are two cases where "redundant" includes
    // should be kept:
    // 1. when the "tail" matches a path exclude filter
    //    (m_excludeFilterRegexpCache)
    // 2. when the explicitly adds a hidden directory, and
    //    we want to index hidden dirs (m_indexHidden)
    bool keepAllIncluded = true;

    auto entry = begin();
    while (entry != end()) {
        if ((*entry).isIncluded && keepAllIncluded) {
            ++entry;
            continue;
        }

        const QString entryPath = (*entry).path;
        auto start = entry; ++start;
        auto parent = std::find_if(start, end(),
            [&entryPath](const FolderConfig& _parent) {
                return entryPath.startsWith(_parent.path);
            });

        if (parent != end()) {
            if ((*entry).isIncluded == (*parent).isIncluded) {
                // remove identical config
                entry = erase(entry);
            } else {
                ++entry;
            }
        } else {
            if (!(*entry).isIncluded) {
                // remove excluded a topmost level (default)
                entry = erase(entry);
            } else {
                ++entry;
            }
        }
    }
}

bool FileIndexerConfig::FolderConfig::operator<(const FolderConfig& other) const
{
    return path.size() > other.path.size() ||
        (path.size() == other.path.size() && path < other.path);
}

bool FileIndexerConfig::FolderCache::addFolderConfig(const FolderConfig& config)
{
    if (config.path.isEmpty()) {
        qCDebug(BALOO) << "Trying to add folder config entry with empty path";
        return false;
    }
    auto newConfig{config};
    newConfig.path = normalizeTrailingSlashes(std::move(newConfig.path));

    auto it = std::lower_bound(cbegin(), cend(), newConfig);
    if (it != cend() && (*it).path == newConfig.path) {
        qCDebug(BALOO) << "Folder config entry for" << newConfig.path << "already exists";
        return false;
    }

    it = insert(it, newConfig);
    return true;
}

void FileIndexerConfig::buildFolderCache()
{
    if (!m_folderCacheDirty) {
        return;
    }

    if (!m_devices) {
        m_devices = new StorageDevices(this);
    }

    FolderCache cache;

    const QStringList includeFolders = m_settings->folders();
    for (const auto& folder : includeFolders) {
        if (!cache.addFolderConfig({folder, true})) {
            qCWarning(BALOO) << "Failed to add include folder config entry for" << folder;
        }
    }

    const QStringList excludeFolders = m_settings->excludedFolders();
    for (const auto& folder : excludeFolders) {
        if (!cache.addFolderConfig({folder, false})) {
            qCWarning(BALOO) << "Failed to add exclude folder config entry for" << folder;
        }
    }

    // Add all removable media and network shares as ignored unless they have
    // been explicitly added in the include list
    const auto allMedia = m_devices->allMedia();
    for (const auto& device: allMedia) {
        const QString mountPath = device.mountPath();
        if (!device.isUsable() && !mountPath.isEmpty()) {
            if (!includeFolders.contains(mountPath)) {
                cache.addFolderConfig({mountPath, false});
            }
        }
    }

    cache.cleanup();
    qCDebug(BALOO) << "Folder cache:" << cache;
    m_folderCache = cache;

    m_folderCacheDirty = false;
}


void FileIndexerConfig::buildExcludeFilterRegExpCache()
{
    QStringList newFilters = excludeFilters();
    m_excludeFilterRegExpCache.rebuildCacheFromFilterList(newFilters);
}

void FileIndexerConfig::buildMimeTypeCache()
{
    const QStringList excludedTypes = m_settings->excludedMimetypes();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    m_excludeMimetypes = excludedTypes.toSet();
#else
    m_excludeMimetypes = QSet<QString>(excludedTypes.begin(), excludedTypes.end());
#endif
}

void FileIndexerConfig::forceConfigUpdate()
{
    m_settings->load();

    m_folderCacheDirty = true;
    buildExcludeFilterRegExpCache();
    buildMimeTypeCache();

    m_indexHidden = m_settings->indexHiddenFolders();
    m_onlyBasicIndexing = m_settings->onlyBasicIndexing();
}

int FileIndexerConfig::databaseVersion() const
{
    return m_settings->dbVersion();
}

void FileIndexerConfig::setDatabaseVersion(int version)
{
    m_settings->setDbVersion(version);
    m_settings->save();
}

bool FileIndexerConfig::indexingEnabled() const
{
    return m_settings->indexingEnabled();
}

uint FileIndexerConfig::maxUncomittedFiles() const
{
    return m_maxUncomittedFiles;
}

} // namespace Baloo
