/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2013-2014 Vishesh Handa <me@vhanda.in>

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

#include "fileindexerconfig.h"
#include "fileexcludefilters.h"
#include "storagedevices.h"

#include <QStringList>
#include <QDir>

#include <QStandardPaths>
#include <KConfigGroup>
#include <QDebug>


namespace
{
/// recursively check if a folder is hidden
bool isDirHidden(QDir& dir)
{
#ifdef __unix__
    return dir.absolutePath().contains(QLatin1String("/."));
#else
    if (QFileInfo(dir.path()).isHidden())
        return true;
    else if (dir.cdUp())
        return isDirHidden(dir);
    else
        return false;
#endif
}

QString stripTrailingSlash(const QString& path)
{
    return path.endsWith('/') ? path.mid(0, path.length()-1) : path;
}

}

using namespace Baloo;

FileIndexerConfig::FileIndexerConfig(QObject* parent)
    : QObject(parent)
    , m_config(QStringLiteral("baloofilerc"))
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


QStringList FileIndexerConfig::includeFolders() const
{
    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    QStringList fl;
    for (int i = 0; i < m_folderCache.count(); ++i) {
        if (m_folderCache[i].second)
            fl << m_folderCache[i].first;
    }
    return fl;
}


QStringList FileIndexerConfig::excludeFolders() const
{
    const_cast<FileIndexerConfig*>(this)->buildFolderCache();

    QStringList fl;
    for (int i = 0; i < m_folderCache.count(); ++i) {
        if (!m_folderCache[i].second)
            fl << m_folderCache[i].first;
    }
    return fl;
}


QStringList FileIndexerConfig::excludeFilters() const
{
    KConfigGroup cfg = m_config.group("General");

    // read configured exclude filters
    QSet<QString> filters = cfg.readEntry("exclude filters", defaultExcludeFilterList()).toSet();

    // make sure we always keep the latest default exclude filters
    // TODO: there is one problem here. What if the user removed some of the default filters?
    if (cfg.readEntry("exclude filters version", 0) < defaultExcludeFilterListVersion()) {
        filters += defaultExcludeFilterList().toSet();

        // write the config directly since the KCM does not have support for the version yet
        // TODO: make this class public and use it in the KCM
        KConfig config(m_config.name());
        KConfigGroup cfg = config.group("General");
        cfg.writeEntry("exclude filters", QStringList::fromSet(filters));
        cfg.writeEntry("exclude filters version", defaultExcludeFilterListVersion());
    }

    // remove duplicates
    return QStringList::fromSet(filters);
}

QStringList FileIndexerConfig::excludeMimetypes() const
{
    return QStringList::fromSet(m_excludeMimetypes);
}

bool FileIndexerConfig::indexHiddenFilesAndFolders() const
{
    return m_indexHidden;
}

bool FileIndexerConfig::onlyBasicIndexing() const
{
    return m_onlyBasicIndexing;
}

bool FileIndexerConfig::isInitialRun() const
{
    return m_config.group("General").readEntry("first run", true);
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
    for (const QPair<QString, bool>& fld: m_folderCache) {
        if (fld.second && fld.first.startsWith(path)) {
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
        if (folder == path)
            return true;

        // check for hidden folders
        QDir dir(path);
        if (!indexHiddenFilesAndFolders() && isDirHidden(dir))
            return false;

        // reset dir, cause isDirHidden modifies the QDir
        dir = path;

        // check the exclude filters for all components of the path
        // after folder
        const QStringList pathComponents = path.mid(folder.count()).split(QLatin1Char('/'), QString::SkipEmptyParts);
        Q_FOREACH (const QString& c, pathComponents) {
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
    if (!indexHiddenFilesAndFolders() && fileName.startsWith('.')) {
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

    const QString p = stripTrailingSlash(path);

    // we traverse the list backwards to catch all exclude folders
    int i = m_folderCache.count();
    while (--i >= 0) {
        const QString& f = m_folderCache[i].first;
        const bool include = m_folderCache[i].second;
        if (p.startsWith(f)) {
            folder = f;
            return include;
        }
    }
    // path is not in the list, thus it should not be included
    folder.clear();
    return false;
}


namespace
{
/**
 * Returns true if the specified folder f would already be excluded using the list
 * folders.
 */
bool alreadyExcluded(const QList<QPair<QString, bool> >& folders, const QString& f)
{
    bool included = false;
    for (int i = 0; i < folders.count(); ++i) {
        QString path = folders[i].first;
        if (!path.endsWith(QLatin1Char('/')))
            path.append(QLatin1Char('/'));

        if (f != folders[i].first && f.startsWith(path)) {
            included = folders[i].second;
        }
    }
    return !included;
}

/**
 * Simple insertion sort
 */
void insertSortFolders(const QStringList& folders, bool include, QList<QPair<QString, bool> >& result)
{
    Q_FOREACH (const QString& f, folders) {
        int pos = 0;
        QString path = stripTrailingSlash(f);
        while (result.count() > pos &&
                result[pos].first < path)
            ++pos;
        result.insert(pos, qMakePair(path, include));
    }
}

/**
 * Remove useless exclude entries which would confuse the folderInFolderList algo.
 * This makes sure all top-level items are include folders.
 * This runs in O(n^2) and could be optimized but what for.
 */
void cleanupList(QList<QPair<QString, bool> >& result)
{
    int i = 0;
    while (i < result.count()) {
        if (result[i].first.isEmpty() ||
                (!result[i].second &&
                 alreadyExcluded(result, result[i].first)))
            result.removeAt(i);
        else
            ++i;
    }
}
}

void FileIndexerConfig::buildFolderCache()
{
    if (!m_folderCacheDirty) {
        return;
    }

    if (!m_devices) {
        m_devices = new StorageDevices(this);
    }

    KConfigGroup group = m_config.group("General");
    QStringList includeFoldersPlain = group.readPathEntry("folders", QStringList() << QDir::homePath());
    QStringList excludeFoldersPlain = group.readPathEntry("exclude folders", QStringList());

    // Add all removable media and network shares as ignored unless they have
    // been explicitly added in the include list
    const auto allMedia = m_devices->allMedia();
    for (const auto& device: allMedia) {
        const QString mountPath = device.mountPath();
        if (!device.isUsable() && !mountPath.isEmpty()) {
            if (!includeFoldersPlain.contains(mountPath)) {
                excludeFoldersPlain << mountPath;
            }
        }
    }

    m_folderCache.clear();
    insertSortFolders(includeFoldersPlain, true, m_folderCache);
    insertSortFolders(excludeFoldersPlain, false, m_folderCache);

    cleanupList(m_folderCache);

    m_folderCacheDirty = false;
}


void FileIndexerConfig::buildExcludeFilterRegExpCache()
{
    QStringList newFilters = excludeFilters();
    m_excludeFilterRegExpCache.rebuildCacheFromFilterList(newFilters);
}

void FileIndexerConfig::buildMimeTypeCache()
{
    m_excludeMimetypes = m_config.group("General").readPathEntry("exclude mimetypes", defaultExcludeMimetypes()).toSet();
}

void FileIndexerConfig::forceConfigUpdate()
{
    m_config.reparseConfiguration();

    m_folderCacheDirty = true;
    buildExcludeFilterRegExpCache();
    buildMimeTypeCache();

    m_indexHidden = m_config.group("General").readEntry("index hidden folders", false);
    m_onlyBasicIndexing = m_config.group("General").readEntry("only basic indexing", false);
}

void FileIndexerConfig::setInitialRun(bool isInitialRun)
{
    m_config.group("General").writeEntry("first run", isInitialRun);
    m_config.sync();
}

bool FileIndexerConfig::initialUpdateDisabled() const
{
    return m_config.group("General").readEntry("disable initial update", true);
}

int FileIndexerConfig::databaseVersion() const
{
    return m_config.group("General").readEntry("dbVersion", 0);
}

void FileIndexerConfig::setDatabaseVersion(int version)
{
    m_config.group("General").writeEntry("dbVersion", version);
    m_config.sync();
}

bool FileIndexerConfig::indexingEnabled() const
{
    return m_config.group("Basic Settings").readEntry("Indexing-Enabled", true);
}

uint FileIndexerConfig::maxUncomittedFiles()
{
    return m_maxUncomittedFiles;
}

